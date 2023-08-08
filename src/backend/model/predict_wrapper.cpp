/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-05 10:00:30
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-18 21:26:06
 * @FilePath: /postgres-kernel/src/backend/model/interface.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "model/model_manager.h"
#include "model/predict_wrapper.h"

#ifdef __cplusplus
#include <sstream>
#include <thread>
#include <vector>
#include "ATen/core/TensorBody.h"

extern "C" {
#endif
#include "postgres.h"

#include "catalog/pg_type_d.h"
#include "fmgr.h"
#include "port.h"
#include "utils/builtins.h"

extern ModelManager model_manager;

VecAggState *
makeVecAggState(FunctionCallInfo fcinfo)
{
    VecAggState *state;
    MemoryContext agg_context;
    MemoryContext old_context;

    if (!AggCheckCallContext(fcinfo, &agg_context))
        elog(ERROR, "aggregate function called in non-aggregate context");

    old_context = MemoryContextSwitchTo(agg_context);

    state = (VecAggState *) palloc0(sizeof(VecAggState));
    state->ctx = agg_context;
    state->model = pstrdup(PG_GETARG_CSTRING(1));
    state->cuda = pstrdup(PG_GETARG_CSTRING(2));

    MemoryContextSwitchTo(old_context);

    return state;
}

Args* 
makeVecFromArgs(FunctionCallInfo fcinfo, int start, int dim) 
{
    Args* vec = (Args*) palloc0(sizeof(Args) * dim);
    for (int i = start; i < dim + start; i++){
        Oid ele_type = get_fn_expr_argtype(fcinfo->flinfo, i);
        switch (ele_type) {
            case INT4OID:
            case INT2OID:
            case INT8OID:
            {
                vec[i - start].integer = PG_GETARG_INT32(i);
                break;
            }
            case FLOAT4OID:
            case FLOAT8OID:
            {
                vec[i - start].integer = PG_GETARG_FLOAT8(i);
                break;
            }
            case TEXTOID:
            {
                vec[i - start].ptr = TextDatumGetCString(PG_GETARG_DATUM(i));
                break;
            }
            case CSTRINGOID:
            {
                vec[i - start].ptr = pstrdup(PG_GETARG_CSTRING(i));
                break;
            }
            case NUMERICOID:
            {
                vec[i - start].floating = DatumGetFloat8(DirectFunctionCall1(numeric_float8, PG_GETARG_DATUM(i)));
                break;
            }
            default:
            {
                ereport(ERROR, (errmsg("%d type don't support!", get_fn_expr_argtype(fcinfo->flinfo, i))));
                break;
            }
        }
    }
    return vec;
}

static bool
wait_and_check_error(std::vector<std::thread> &pool, std::vector<int> &res, int parallel_num)
{
    bool has_error = false;
    for (auto &t : pool)
            t.join();

    for (int i = 0; i < parallel_num; i++)
    {
            if (!res[i])
            {
                has_error = true;
                break;
            }
    }

    pool.clear();
    res.clear();
    return has_error;
}


#define CLEAN_UP_CPP_OBJS() \
input_tensors.clear();      \
input_batch_tensor.clear(); \
output.~IValue();    \
outputs.clear()

#define WAIT_AND_CHECK_ERROR(stage)                            \
if (wait_and_check_error(pool, res, prcsd_batch_n))            \
{                                                              \
    CLEAN_UP_CPP_OBJS();                                        \
    ereport(ERROR, (errmsg("meet error in " stage " stage"))); \
}

#define CLOCK_START() auto start = std::chrono::system_clock::now()
#define CLOCK_END(type) state->type##_time += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start).count()

static std::vector<torch::jit::IValue>
split_results_one_level(torch::jit::IValue res) {
    std::vector<torch::jit::IValue> rets;
    if (res.isTensor()) {
        auto&& tensors = torch::split(res.toTensor(), 1, 0);
        for (auto&& t: tensors) {
            rets.emplace_back(torch::jit::IValue(std::move(t)));
        }
    } else if (res.isList()) {
        auto&& list = res.toList();
        for (auto&& it: list) {
            rets.emplace_back(std::move(it));
        }
    } else if (res.isTuple()) {
        auto&& eles = res.toTuple()->elements();
        for (auto&& e : eles) {
            rets.emplace_back(torch::jit::IValue(std::move(c10::ivalue::Tuple::create(e))));
        }
    }
    return rets;
}

static std::vector<torch::jit::IValue>
split_results(torch::jit::IValue output) {
    std::vector<torch::jit::IValue> ret;
    if (output.isTensor()) {
        return split_results_one_level(output);
    } else if (output.isTuple()) {
        std::vector<std::vector<torch::jit::IValue>> cols;
        for (auto&& it : output.toTuple()->elements()) {
            cols.emplace_back(split_results_one_level(std::move(it)));
        }
        // col to row
        int row_num = cols[0].size();
        std::vector<torch::jit::IValue> row;
        for (int i = 0; i < row_num; i++) {
            for (unsigned int j = 0; j < cols.size(); j++) {
                row.emplace_back(std::move(cols[j][i]));
            }
            ret.emplace_back(torch::jit::IValue(c10::ivalue::Tuple::create(std::move(row))));
        }
        return ret;
    } 
    return ret;
}


void
infer_batch_internal(VecAggState *state, bool ret_float8)
{
    char* model_path = nullptr;
    int prcsd_batch_n = state->ins->length;
    
    if(strlen(state->model) == 0){
        ereport(ERROR, (errmsg("model name is empty!")));
    }

    if(!model_manager_get_model_path(&model_manager, state->model, &model_path)){
        ereport(ERROR, (errmsg("model not exist,can't get path!")));
    }

    // 1. 加载模型
    if(!model_manager_load_model(&model_manager, model_path)){
        ereport(ERROR, (errmsg("load model error")));
    }
    
    // 2. 设置gpu模式
    if(pg_strcasecmp(state->cuda, "gpu") == 0 && 
       model_manager_set_cuda(&model_manager, model_path)){
    }
 
    std::vector<std::thread> pool;
    std::vector<int> res(prcsd_batch_n, 0);
    std::vector<std::vector<torch::jit::IValue>> input_tensors(prcsd_batch_n, std::vector<torch::jit::IValue>());
    //std::vector<std::vector<at::Tensor>> batch_inputs_tmp;
    std::vector<torch::jit::IValue> input_batch_tensor;
    torch::jit::IValue output;
    std::vector<torch::jit::IValue> outputs; // batch of tuples<tensor|list|tuple> or tensors

    // 3. 输入预处理
    {
        CLOCK_START();

        for (int i = 0; i < prcsd_batch_n; i++) {
            pool.emplace_back([&, i](){
                Args* in = (Args*)list_nth(state->ins, i);
                res[i] = model_manager_pre_process(&model_manager, model_path, input_tensors[i], in);
            });
        }
        WAIT_AND_CHECK_ERROR("preprocess");

        CLOCK_END(pre);
    }

    // 4. 预测
    {
        CLOCK_START();

        int each_input_tensor_size = (input_tensors.size() != 0 ? input_tensors[0].size() : 0);
        input_batch_tensor.resize(each_input_tensor_size);
        for(int i = 0; i < each_input_tensor_size; ++i) {
            std::vector<at::Tensor> col_tensors(prcsd_batch_n);
            for(int j = 0; j < prcsd_batch_n; ++j){
                col_tensors[j] = input_tensors[j][i].toTensor();
            }
            input_batch_tensor[i] = (torch::concat(col_tensors, 0));
        }

        if(!model_manager_predict_multi_input(&model_manager, model_path, input_batch_tensor, output)) {
            CLEAN_UP_CPP_OBJS();
            ereport(ERROR, (errmsg("%s:predict error!", model_path)));
        }

        CLOCK_END(infer);
    }
    

    // 5. 结果处理
    {
        CLOCK_START();

        outputs = split_results(output);
        if (outputs.empty()) {
            CLEAN_UP_CPP_OBJS();
            ereport(ERROR, (errmsg("cannot handle the result type from model!")));
        }

        for (int i = 0; i < prcsd_batch_n; i++)
            state->outs = lappend(state->outs, palloc0(sizeof(Args)));

        for (int i = 0; i < prcsd_batch_n; i++) {
            pool.emplace_back([&, i](){
                Args* in = (Args*)list_nth(state->ins, i);
                torch::jit::IValue wrapped_out(outputs[i]);
                if (ret_float8) {
                    float8& out = ((Args*)list_nth(state->outs, i))->floating;
                    res[i] = model_manager_output_process_float(&model_manager, model_path, wrapped_out, in, out);
                } else {
                    std::string result_str;
                    res[i] = model_manager_output_process_text(&model_manager, model_path, wrapped_out, in, result_str);   
                    ((Args*)list_nth(state->outs, i))->ptr = pstrdup(result_str.c_str());
                }
            });
        }
        WAIT_AND_CHECK_ERROR("postprocess");

        CLOCK_END(post);
    }

    /* update infered batch size */
    state->prcsd_batch_n = prcsd_batch_n;
    state->batch_i++;
}

float8 
predict_float(const char* model_name, const char* cuda, Args* args)
{
    char* model_path = nullptr;
    std::vector<torch::jit::IValue> input_tensor;
    torch::jit::IValue output_tensor;

    int64_t total_time = 0;
    int64_t pre_time = 0;
    int64_t predict_time = 0;
    int64_t after_time = 0;
    
    if(strlen(model_name) == 0){
        ereport(ERROR, (errmsg("model name is empty!")));
    }

    if(!model_manager_get_model_path(&model_manager, model_name, &model_path)){
        ereport(ERROR, (errmsg("model not exist,can't get path!")));
    }
    
    //ereport(INFO, (errmsg("model_path:%s", model_path)));

    
    // 1. 加载模型
    if(!model_manager_load_model(&model_manager, model_path)){
        ereport(ERROR, (errmsg("load model error")));
    }
    
    if(pg_strcasecmp(cuda, "gpu") == 0 && 
       model_manager_set_cuda(&model_manager, model_path)){

    }
    // 2. 设置gpu模式
    // if(model_manager_set_cuda(&model_manager, model_path)){
        
    // }
    // 3. 输入预处理
    auto start_time = std::chrono::system_clock::now();
    if(!model_manager_pre_process(&model_manager, model_path, input_tensor, args)){
        ereport(ERROR, (errmsg("%s:preprocess error!", model_path)));
    } 
    pre_time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time).count();

    // 4. 预测
    start_time = std::chrono::system_clock::now();
    if(!model_manager_predict_multi_input(&model_manager, model_path, input_tensor, output_tensor)){
        ereport(ERROR, (errmsg("%s:predict error!", model_path)));
    }
    predict_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time).count();

    // 5. 结果处理
    float8 result;
    start_time = std::chrono::system_clock::now();
    if(!model_manager_output_process_float(&model_manager, model_path, output_tensor, args, result)){
        ereport(ERROR, (errmsg("%s OutputProcessFloat callback is empty!", model_path)));
    }
    after_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time).count();

    total_time = pre_time + predict_time + after_time;

    if(Debug_print_batch_time == true){
        ereport(NOTICE, 
                (errmsg(" pre process: %ld ms(%.2f%%)\n"
                        " infer: %ld ms(%.2f%%)\n"
                        " post process: %ld ms(%.2f%%)",
                        pre_time, (pre_time / (float)total_time) * 100, 
                        predict_time, (predict_time / (float)total_time)  * 100, 
                        after_time, (after_time / (float)total_time  * 100))));
    }

    // ereport(INFO, (errmsg("after: %f", result)));
    return result;
}

text*  
predict_text(const char* model_name, const char* cuda, Args* args)
{
    char* model_path = nullptr;
    std::vector<torch::jit::IValue> input_tensor;
    torch::jit::IValue output_tensor;

    int64_t total_time = 0;
    int64_t pre_time = 0;
    int64_t predict_time = 0;
    int64_t after_time = 0;
    
    if(strlen(model_name) == 0){
        ereport(ERROR, (errmsg("model name is empty!")));
    }

    if(!model_manager_get_model_path(&model_manager, model_name, &model_path)){
        ereport(ERROR, (errmsg("model not exist,can't get path!")));
    }
    
    //ereport(INFO, (errmsg("model_path:%s", model_path.c_str())));

    
    // 1. 加载模型
    if(!model_manager_load_model(&model_manager, model_path)){
        ereport(ERROR, (errmsg("load model error")));
    }
    
    // 2. 设置gpu模式
    if(pg_strcasecmp(cuda, "gpu") == 0 && 
       model_manager_set_cuda(&model_manager, model_path)){

    }

    // 3. 输入预处理
    auto start_time = std::chrono::system_clock::now();
    if(!model_manager_pre_process(&model_manager, model_path, input_tensor, args)){
        ereport(ERROR, (errmsg("%s:preprocess error!", model_path)));
    }
    pre_time  = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time).count();

    // 4. 预测
    start_time = std::chrono::system_clock::now();
    if(!model_manager_predict_multi_input(&model_manager, model_path, input_tensor, output_tensor)){
        ereport(ERROR, (errmsg("%s:predict error!", model_path)));
    }
    predict_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time).count();
    // 5. 结果处理
    text* result = nullptr;
    std::string result_str;
    start_time = std::chrono::system_clock::now();
    if(!model_manager_output_process_text(&model_manager, model_path, output_tensor, args, result_str)){
        ereport(ERROR, (errmsg("%s OutputProcessFloat callback is empty!", model_path)));
    }
    after_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time).count();

    total_time = pre_time + predict_time + after_time;

    if(Debug_print_batch_time == true){
        ereport(NOTICE, 
                (errmsg(" pre process: %ld ms(%.2f%%)\n"
                        " infer: %ld ms(%.2f%%)\n"
                        " post process: %ld ms(%.2f%%)",
                        pre_time, (pre_time / (float)total_time) * 100, 
                        predict_time, (predict_time / (float)total_time)  * 100, 
                        after_time, (after_time / (float)total_time  * 100))));
    }

    result = (text*)palloc(result_str.size() + VARHDRSZ);
    SET_VARSIZE(result, result_str.size() + VARHDRSZ);
    memcpy(VARDATA(result), result_str.c_str(), result_str.size());

    return result;
}




#ifdef __cplusplus
}
#endif