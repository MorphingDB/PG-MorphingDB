#ifndef VECTOR_TENSOR_H
#define VECTOR_TENSOR_H

#include <torch/torch.h>
#include <torch/script.h>

#include "vector.h"

extern "C" {

Vector* tensor_to_vector(torch::Tensor& tensor);

torch::Tensor vector_to_tensor(Vector& vector);


}
#endif