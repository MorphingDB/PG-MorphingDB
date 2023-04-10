#include <iostream>
#include <torch/torch.h>
#include <torch/script.h>

#include "model/libtorch_wrapper.h"

static torch::jit::script::Module torch_model;

void load_torch_model(const char* model_path) {
    try {
        torch_model = torch::jit::load(model_path);
        std::cout << "Model loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading the model: " << e.what() << std::endl;
        throw;
    }
}

float predict_with_torch_model(int input1, int input2) {
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(torch::tensor({input1, input2}));

    at::Tensor output = torch_model.forward(inputs).toTensor();
    return output.item<float>();
}
