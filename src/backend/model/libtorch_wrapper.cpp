#include <iostream>
#include <torch/torch.h>
#include <torch/script.h>

#include "model/libtorch_wrapper.h"

static torch::jit::script::Module torch_model;

unsigned load_torch_model(const char* model_path) {
    try {
        torch_model = torch::jit::load(model_path);
        // std::cout << "Model loaded successfully" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        // std::cerr << "Error loading the model: " << e.what() << std::endl;
        // throw;
        return 1;
    }
}

float predict_with_torch_model() {
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(torch::ones({1,3,224,224}));


    at::Tensor output = torch_model.forward(inputs).toTensor();
    return *output.data_ptr<float>();
}
