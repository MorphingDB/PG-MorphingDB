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
extern "C" {
#endif
#include "postgres.h"
#include "fmgr.h"
#include "port.h"

extern ModelManager model_manager;

float8 
predict_float(const char* model_name, const char* cuda, Args* args)
{
    char* model_path = nullptr;
    std::vector<torch::jit::IValue> input_tensor;
    torch::jit::IValue output_tensor;
    
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
    if(!model_manager_pre_process(&model_manager, model_path, input_tensor, args)){
        ereport(ERROR, (errmsg("%s:preprocess error!", model_path)));
    } 
    // 4. 预测
    if(!model_manager_predict_multi_input(&model_manager, model_path, input_tensor, output_tensor)){
        ereport(ERROR, (errmsg("%s:predict error!", model_path)));
    }
    // 5. 结果处理
    float8 result;
    if(!model_manager_output_process_float(&model_manager, model_path, output_tensor, args, result)){
        ereport(ERROR, (errmsg("%s OutputProcessFloat callback is empty!", model_path)));
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
    if(!model_manager_pre_process(&model_manager, model_path, input_tensor, args)){
        ereport(ERROR, (errmsg("%s:preprocess error!", model_path)));
    }
    // 4. 预测
    if(!model_manager_predict_multi_input(&model_manager, model_path, input_tensor, output_tensor)){
        ereport(ERROR, (errmsg("%s:predict error!", model_path)));
    }
    // 5. 结果处理
    text* result = nullptr;
    std::string result_str;
    if(!model_manager_output_process_text(&model_manager, model_path, output_tensor, args, result_str)){
        ereport(ERROR, (errmsg("%s OutputProcessFloat callback is empty!", model_path)));
    }

    result = (text*)palloc(result_str.size() + VARHDRSZ);
    SET_VARSIZE(result, result_str.size() + VARHDRSZ);
    memcpy(VARDATA(result), result_str.c_str(), result_str.size());

    return result;
}




#ifdef __cplusplus
}
#endif