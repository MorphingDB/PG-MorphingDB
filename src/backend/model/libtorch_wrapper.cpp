#include <iostream>
#include <torch/torch.h>
#include <torch/script.h>

// #include "utils/elog.h"
#include "model/libtorch_wrapper.h"

static std::unordered_map<std::string, torch::jit::script::Module> models;

bool loadTorchModel(const char* modelName, const char* modelPath) {
    try {
        torch::jit::script::Module model = torch::jit::load(modelPath);
        models[modelName] = model;
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}


void unloadTorchModel(const char* modelName) {
    auto md = models.find(modelName);
    if(md != models.end()) {
        models.erase(md);
    }
}

void debug_models(){
    // for(auto &[k, v] : models) {
    //     printf("model name : %s \n", k.c_str());
    //     // elog(INFO, "model name : %s\n", k) ;
    // }
}

float predictWithTorchModel(const char* modelName) {
    auto md = models.find(modelName);
    if(md == models.end()) {
        return 0.0;
    }

    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(torch::ones({1,3,224,224}));

    at::Tensor output = md->second.forward(inputs).toTensor();
    return *output.data_ptr<float>();
}
