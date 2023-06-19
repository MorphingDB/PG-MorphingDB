
#include "model/model_manager.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "postgres.h"
#include "utils/relcache.h"
#include "utils/syscache.h"
#include "utils/builtins.h"
#include "utils/rel.h"
#include "storage/lockdefs.h"
#include "access/table.h"
#include "access/htup_details.h"
#include "catalog/pg_model_info.h"
#include "utils/memutils.h"

ModelManager model_manager;

bool 
model_manager_load_model(ModelManager *manager, const char *model_path)
{
    if(manager->module_handle_.find(model_path) != manager->module_handle_.end()){
        return true;
    }

    try {
        torch::jit::script::Module cur_module = torch::jit::load(model_path);
        manager->module_handle_[model_path].first = cur_module;
        manager->module_handle_[model_path].second = false;
        cur_module.eval();
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

bool 
model_manager_get_model_path(ModelManager *manager, const char *model_name, char **model_path)
{
    Relation            pg_model_info_rel; 
    HeapTuple	        tuple;
    Datum               path_datum;
    bool                path_is_null;
    pg_model_info_rel = table_open(ModelInfoRelationId, AccessShareLock);
    tuple = SearchSysCache1(MODELNAMEVERSION, CStringGetDatum(model_name));

    Assert(tuple != NULL);

    if(!HeapTupleIsValid(tuple)){
        ereport(ERROR,
                (errcode(ERRCODE_DUPLICATE_MODEL),
                 errmsg("model \"%s\" not exists", model_name)));
        return false;
    }

    path_datum = heap_getattr(tuple, Anum_pg_model_info_modelpath, 
                              RelationGetDescr(pg_model_info_rel), &path_is_null);

    if(!path_is_null) *model_path = TextDatumGetCString(path_datum);

    ReleaseSysCache(tuple);
    table_close(pg_model_info_rel, AccessShareLock);

    return true;
}

bool 
model_manager_get_model_md5(ModelManager *manager, const char *model_path, char **md5)
{
    Relation            pg_model_info_rel; 
    HeapTuple	        tuple;
    Datum               md5_datum;
    bool                md5_is_null;

    if(!SearchSysCacheExists1(Anum_pg_model_info_modelname, CStringGetDatum(model_path))){
        ereport(ERROR,
                (errcode(ERRCODE_DUPLICATE_MODEL),
                 errmsg("model \"%s\" not exists", model_path)));
        return false;
    }

    pg_model_info_rel = table_open(ModelInfoRelationId, AccessShareLock);

    tuple = SearchSysCache1(Anum_pg_model_info_modelpath, CStringGetDatum(model_path));

    Assert(tuple != NULL);

    md5_datum = heap_getattr(tuple, Anum_pg_model_info_md5, 
                              RelationGetDescr(pg_model_info_rel), &md5_is_null);

    if(!md5_is_null) *md5 = TextDatumGetCString(md5_datum);

    table_close(pg_model_info_rel, AccessShareLock);

    return true;
}

bool 
model_manager_set_cuda(ModelManager *manager, const char *model_path)
{
    if(manager->module_handle_.find(model_path) != manager->module_handle_.end()){
        if(manager->module_handle_[model_path].second == true){
            ereport(INFO, (errmsg("no need to to KCUDA!")));
            return true;
        } else {
            if(torch::cuda::is_available()){
                manager->module_handle_[model_path].first.to(at::kCUDA);
                manager->module_handle_[model_path].second = true;
                return true;
            }
            return false;
        }
    } else {
        return false;
    }
}

bool 
model_manager_is_cuda(ModelManager *manager, const char *model_path)
{
    if(manager->module_handle_.find(model_path) == manager->module_handle_.end()){
        return false;
    }
    return manager->module_handle_[model_path].second;
}

bool 
model_manager_pre_process(ModelManager *manager, const char *model_path, std::vector<torch::jit::IValue>& img_tensor, Args *args)
{
    if(manager->module_preprocess_functions_.find(model_path) != manager->module_preprocess_functions_.end()){
        return manager->module_preprocess_functions_[model_path](img_tensor, args);
    }
    return false;
}

bool 
model_manager_output_process_float(ModelManager *manager, const char *model_path, at::Tensor& output_tensor, float8& result)
{
    if(manager->module_outputprocess_functions_float_.find(model_path) != manager->module_outputprocess_functions_float_.end()){
        return manager->module_outputprocess_functions_float_[model_path](output_tensor, result);
    }

    return false;
}

bool 
model_manager_output_process_text(ModelManager *manager, const char *model_path, at::Tensor& output_tensor, text *result)
{
    if(manager->module_outputprocess_functions_text_.find(model_path) != manager->module_outputprocess_functions_text_.end()){
        //ereport(INFO, (errmsg("user output_text function")));
        return manager->module_outputprocess_functions_text_[model_path](output_tensor, result);
    }
    // auto max_result = output_tensor.max(1,true);
    // auto max_index = std::get<1>(max_result).item<float>();
    // char buffer[50];
    // sprintf(buffer, "%f", max_index);
    // result = cstring_to_text("12123");
    return false;
}

void 
model_manager_register_pre_process(ModelManager *manager, const char *model_name, PreProcessCallback func)
{
    char* model_path = nullptr;
    if(model_manager_get_model_path(manager, model_name, &model_path)){
        // ereport(INFO, (errmsg("model_path:%s", model_path)));
        // ereport(INFO, (errmsg("ponit1:%p", manager)));
        manager->module_preprocess_functions_[model_path] = func;
        return;
    }else{
        ereport(ERROR, (errmsg("model:%s not exist!", model_name)));
    }
}

void 
model_manager_register_output_process_float(ModelManager *manager, const char *model_name, OutputProcessFloatCallback func)
{
    char* model_path = nullptr;
    if(model_manager_get_model_path(manager, model_name, &model_path)){
        manager->module_outputprocess_functions_float_[model_path] = func;
        return;
    }else{
        ereport(ERROR, (errmsg("model:%s not exist!", model_name)));
    }
}

void 
model_manager_register_output_process_text(ModelManager *manager, const char *model_name, OutputProcessTextCallback func)
{
    char* model_path = nullptr;
    if(model_manager_get_model_path(manager, model_name, &model_path)){
        manager->module_outputprocess_functions_text_[model_path] = func;
        return;
    }else{
        ereport(ERROR, (errmsg("model:%s not exist!", model_name)));
    }
}

bool 
model_manager_predict(ModelManager *manager, const char *model_path, at::Tensor& input, at::Tensor& output)
{
    if(manager->module_handle_.find(model_path) == manager->module_handle_.end()){
        return false;
    }
    try {
        output = manager->module_handle_[model_path].first.forward({input}).toTensor();
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

bool 
model_manager_predict_multi_input(ModelManager *manager, const char *model_path, std::vector<torch::jit::IValue>& input, at::Tensor& output)
{
    if(manager->module_handle_.find(model_path) == manager->module_handle_.end()){
        return false;
    }
    try {
        output = manager->module_handle_[model_path].first.forward(input).toTensor();
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

#ifdef __cplusplus
}
#endif
