#ifndef LIBTORCH_WRAPPER_H
#define LIBTORCH_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

bool loadTorchModel(const char* modelName, const char* modelPath);
void unloadTorchModel(const char* modelName);
float predictWithTorchModel(const char* modelName);
void debug_models();

#ifdef __cplusplus
}
#endif

#endif