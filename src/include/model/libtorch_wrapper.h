#ifndef LIBTORCH_WRAPPER_H
#define LIBTORCH_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void load_torch_model(const char* model_path);
float predict_with_torch_model(int input1, int input2);

#ifdef __cplusplus
}
#endif

#endif