
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

// //内核部分不需要使用spi这种连接方式去修改pg表中得数据
// //内置的模型注册回调函数可以在构造函数中
ModelManager::ModelManager()
{
    
}

ModelManager::~ModelManager()
{
    
}



bool ModelManager::GetModelPath(const std::string model_name, std::string& model_path)
{   
    Relation            pg_model_info_rel; 
    HeapTuple	        tuple;
    Datum               path_datum;
    bool                path_is_null;
    
    pg_model_info_rel = table_open(ModelInfoRelationId, AccessShareLock);
    tuple = SearchSysCache1(MODELNAMEVERSION, CStringGetDatum(model_name.c_str()));
    if(!HeapTupleIsValid(tuple)){
        ereport(ERROR,
                (errcode(ERRCODE_DUPLICATE_MODEL),
                 errmsg("model \"%s\" not exists", model_name.c_str())));
        return false;
    }

    path_datum = heap_getattr(tuple, Anum_pg_model_info_modelpath, 
                              RelationGetDescr(pg_model_info_rel), &path_is_null);

    if(!path_is_null) model_path = TextDatumGetCString(path_datum);
    //ereport(INFO, (errmsg("model_path:%s", model_path.c_str())));

    // 释放资源
    ReleaseSysCache(tuple);
    table_close(pg_model_info_rel, AccessShareLock);

    return true;
}

bool ModelManager::GetModelMd5(const std::string model_path, std::string& md5)
{
    Relation            pg_model_info_rel; 
    HeapTuple	        tuple;
    Datum               md5_datum;
    bool                md5_is_null;

    if(!SearchSysCacheExists1(Anum_pg_model_info_modelname, CStringGetDatum(model_path.c_str()))){
        ereport(ERROR,
                (errcode(ERRCODE_DUPLICATE_MODEL),
                 errmsg("model \"%s\" not exists", model_path.c_str())));
        return false;
    }

    pg_model_info_rel = table_open(ModelInfoRelationId, AccessShareLock);

    tuple = SearchSysCache1(Anum_pg_model_info_modelpath, CStringGetDatum(model_path.c_str()));

    Assert(tuple != NULL);

    md5_datum = heap_getattr(tuple, Anum_pg_model_info_md5, 
                              RelationGetDescr(pg_model_info_rel), &md5_is_null);

    if(!md5_is_null) md5 = TextDatumGetCString(md5_datum);

    table_close(pg_model_info_rel, AccessShareLock);

    return true;
}

bool ModelManager::LoadModel(std::string model_path)
{
    if(module_handle_.find(model_path) != module_handle_.end()){
        return true;
    }
    // 加载model前需要先校验本地文件系统的md5值与表中的md5值，防止篡改
    // std::string table_md5, file_md5;
    // Md5Sum(model_path, file_md5);
    // GetModelMd5(model_path, table_md5);
    // if(file_md5 != table_md5){
    //     return false;
    // }

    try {
        torch::jit::script::Module cur_module = torch::jit::load(model_path.c_str());
        module_handle_[model_path].first = cur_module;
        module_handle_[model_path].second = false;
        cur_module.eval();
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

bool ModelManager::SetCuda(const std::string& model_path)
{
    if(module_handle_.find(model_path) != module_handle_.end()){
        if(module_handle_[model_path].second == true){
            ereport(INFO, (errmsg("no need to to KCUDA!")));
            return true;
        } else {
            if(torch::cuda::is_available()){
                module_handle_[model_path].first.to(at::kCUDA);
                module_handle_[model_path].second = true;
                return true;
            }
            return false;
        }
    } else {
        return false;
    }
}

bool ModelManager::IsCuda(const std::string& model_path)
{
    if(module_handle_.find(model_path) == module_handle_.end()){
        return false;
    }
    return module_handle_[model_path].second;
}

bool ModelManager::PreProcess(const std::string model_path, std::vector<torch::jit::IValue>& img_tensor, Args* args)
{
    if(module_preprocess_functions_.find(model_path) != module_preprocess_functions_.end()){
        //ereport(INFO, (errmsg("user preprocess function")));
        return module_preprocess_functions_[model_path](img_tensor, args);
    }
    //img_tensor = torch::ones({1,3,224,224});
    return false;
}

bool ModelManager::OutputProcessText(const std::string model_path, at::Tensor& output_tensor, text* result)
{
    if(module_outputprocess_functions_text_.find(model_path) != module_outputprocess_functions_text_.end()){
        //ereport(INFO, (errmsg("user output_text function")));
        return module_outputprocess_functions_text_[model_path](output_tensor, result);
    }
    // auto max_result = output_tensor.max(1,true);
    // auto max_index = std::get<1>(max_result).item<float>();
    // char buffer[50];
    // sprintf(buffer, "%f", max_index);
    // result = cstring_to_text("12123");
    return false;
}

bool ModelManager::OutputProcessFloat(const std::string model_path, at::Tensor& output_tensor, float8& result)
{
    if(module_outputprocess_functions_float_.find(model_path) != module_outputprocess_functions_float_.end()){
        return module_outputprocess_functions_float_[model_path](output_tensor, result);
    }

    return false;
}


void ModelManager::RegisterPreProcess(const std::string& model_name, PreProcessCallback func)
{
    std::string model_path;
    if(GetModelPath(model_name, model_path)){
        module_preprocess_functions_[model_path] = func;
        return;
    }else{
        ereport(ERROR, (errmsg("model:%s not exist!", model_name.c_str())));
    }
}

void ModelManager::RegisterOutoutProcessFloat(const std::string& model_name, OutputProcessFloatCallback func)
{
    std::string model_path;
    if(GetModelPath(model_name, model_path)){
        module_outputprocess_functions_float_[model_path] = func;
        return;
    }else{
        ereport(ERROR, (errmsg("model:%s not exist!", model_name.c_str())));
    }
}

void ModelManager::RegisterOutoutProcessText(const std::string& model_name, OutputProcessTextCallback func)
{
    std::string model_path;
    if(GetModelPath(model_name, model_path)){
        module_outputprocess_functions_text_[model_path] = func;
        return;
    }else{
        ereport(ERROR, (errmsg("model:%s not exist!", model_name.c_str())));
    }
}

bool ModelManager::Predict(const std::string& model_path, at::Tensor& input, at::Tensor& output)
{
    if(module_handle_.find(model_path) == module_handle_.end()){
        return false;
    }
    try {
        output = module_handle_[model_path].first.forward({input}).toTensor();
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

bool ModelManager::Predict(const std::string& model_path, std::vector<torch::jit::IValue>& input, at::Tensor& output)
{
    if(module_handle_.find(model_path) == module_handle_.end()){
        return false;
    }
    try {
        output = module_handle_[model_path].first.forward(input).toTensor();
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

#ifdef __cplusplus
}
#endif
