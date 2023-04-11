#ifndef LIBTORCH_WRAPPER_H
#define LIBTORCH_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned load_torch_model(const char* model_path);
float predict_with_torch_model();

#ifdef __cplusplus
}
#endif

#endif