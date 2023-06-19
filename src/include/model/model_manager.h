/*
 * @Author: laihuihang laihuihang@foxmail.com
 * @Date: 2023-06-16 14:03:32
 * @LastEditors: laihuihang laihuihang@foxmail.com
 * @LastEditTime: 2023-06-19 15:44:58
 * @FilePath: /postgres-kernel-new/src/include/model/model_manager.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_

#include <torch/torch.h>
#include <torch/script.h>

extern "C"{

#include "model_define.h"
#include <unordered_map>

using PreProcessCallback = bool(*)(std::vector<torch::jit::IValue>&, Args*);
using OutputProcessFloatCallback = bool(*)(at::Tensor&, float8&);
using OutputProcessTextCallback = bool(*)(at::Tensor&, text*);

typedef struct ModelManager {
    std::unordered_map<std::string, std::pair<torch::jit::script::Module, bool>>        module_handle_;  //key为路径，value为module句柄以及是否使用gpu
    std::unordered_map<std::string, PreProcessCallback>                                 module_preprocess_functions_; //key为模型路径，value为注册的预处理回调函数
    std::unordered_map<std::string, OutputProcessFloatCallback>                         module_outputprocess_functions_float_; //key为模型路径，value为输出处理回调函数
    std::unordered_map<std::string, OutputProcessTextCallback>                          module_outputprocess_functions_text_; //key为模型路径，value为输出处理回调函数
}ModelManager;


// ... 其他函数声明 ...

bool model_manager_load_model(ModelManager *manager, const char *model_path);

bool model_manager_get_model_path(ModelManager *manager, const char *model_name, char **model_path);

bool model_manager_get_model_md5(ModelManager *manager, const char *model_path, char **md5);

bool model_manager_set_cuda(ModelManager *manager, const char *model_path);

bool model_manager_is_cuda(ModelManager *manager, const char *model_path);

bool model_manager_pre_process(ModelManager *manager, const char *model_path, std::vector<torch::jit::IValue>& img_tensor, Args *args);

bool model_manager_output_process_float(ModelManager *manager, const char *model_path, at::Tensor& output_tensor, float8& result);

bool model_manager_output_process_text(ModelManager *manager, const char *model_path, at::Tensor& output_tensor, text* result);

void model_manager_register_pre_process(ModelManager *manager, const char *model_name, PreProcessCallback func);

void model_manager_register_output_process_float(ModelManager *manager, const char *model_name, OutputProcessFloatCallback func);

void model_manager_register_output_process_text(ModelManager *manager, const char *model_name, OutputProcessTextCallback func);

bool model_manager_predict(ModelManager *manager, const char *model_path, at::Tensor& input, at::Tensor& output);

bool model_manager_predict_multi_input(ModelManager *manager, const char *model_path, std::vector<torch::jit::IValue>& input, at::Tensor& output);

}
#endif // _MODEL_MANAGER_H_