#include "utils/vector_tensor.h"
#include "c.h"

extern "C"{

Vector* 
tensor_to_vector(torch::Tensor& tensor)
{
    uint32 dim = tensor.dim();
    uint32 shape_size = tensor.sizes().size();


    Vector* vector = new_vector(dim, shape_size);

    for (uint32 i=0; i<shape_size; ++i) {
        vector->shape[i] = tensor.size(i);
    }

    torch::Tensor flattened_tensor = tensor.view({-1});
    auto accessor = flattened_tensor.accessor<float, 1>();
    for (int i = 0; i < flattened_tensor.numel(); ++i) {
        vector->x[i] = accessor[i];
    }

    return vector;
}

torch::Tensor 
vector_to_tensor(Vector& vector)
{
    torch::TensorOptions options = torch::TensorOptions().dtype(torch::kFloat32);
    std::vector<int64_t> shape(vector.shape, vector.shape + vector.shape_size);
    torch::Tensor tensor = torch::from_blob(vector.x, shape, options).clone();
    return tensor;
}

}