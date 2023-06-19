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
    
    // 2. 设置gpu模式
    if (!model_manager_is_cuda(&model_manager, model_path) && 
        model_manager_set_cuda(&model_manager, model_path)){
        ereport(INFO, (errmsg("libtorch use gpu!")));
    }

    // 3. 输入预处理
    std::vector<torch::jit::IValue> img_tensor;
    at::Tensor output_tensor;
    if(!model_manager_pre_process(&model_manager, model_path, img_tensor, args)){
        ereport(ERROR, (errmsg("preprocess error!")));
    }
    // 4. 预测
    if(!model_manager_predict_multi_input(&model_manager, model_path, img_tensor, output_tensor)){
        ereport(ERROR, (errmsg("predict error!")));
    }
    // 5. 结果处理
    float8 result;
    if(!model_manager_output_process_float(&model_manager, model_path, output_tensor, result)){
        ereport(ERROR, (errmsg("%s OutputProcessFloat callback is empty!", model_path)));
    }
    // ereport(INFO, (errmsg("after: %f", result)));
    return result;
}

text*  
predict_text(const char* model_name, const char* cuda, Args* args)
{
    char* model_path = nullptr;
    
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
    if (!model_manager_is_cuda(&model_manager, model_path) && 
        model_manager_set_cuda(&model_manager, model_path)){
        ereport(INFO, (errmsg("libtorch use gpu!")));
    }

    // 3. 输入预处理
    std::vector<torch::jit::IValue> img_tensor;
    at::Tensor output_tensor;
    if(!model_manager_pre_process(&model_manager, model_path, img_tensor, args)){
        ereport(ERROR, (errmsg("preprocess error!")));
    }
    // 4. 预测
    if(!model_manager_predict_multi_input(&model_manager, model_path, img_tensor, output_tensor)){
        ereport(ERROR, (errmsg("predict error!")));
    }
    // 5. 结果处理
    text* result;
    if(!model_manager_output_process_text(&model_manager, model_path, output_tensor, result)){
        ereport(ERROR, (errmsg("%s OutputProcessFloat callback is empty!", model_path)));
    }
    return result;
}


#ifdef __cplusplus
}
#endif