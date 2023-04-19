#pragma once

// @generated by torchgen/gen.py from Function.h

#include <ATen/Context.h>
#include <ATen/DeviceGuard.h>
#include <ATen/TensorUtils.h>
#include <ATen/TracerMode.h>
#include <ATen/core/Generator.h>
#include <ATen/core/Reduction.h>
#include <ATen/core/Tensor.h>
#include <c10/core/Scalar.h>
#include <c10/core/Storage.h>
#include <c10/core/TensorOptions.h>
#include <c10/util/Deprecated.h>
#include <c10/util/Optional.h>



#include <ATen/ops/mkldnn_linear_backward_input_ops.h>

namespace at {


// aten::mkldnn_linear_backward_input(int[] input_size, Tensor grad_output, Tensor weight) -> Tensor
inline at::Tensor mkldnn_linear_backward_input(at::IntArrayRef input_size, const at::Tensor & grad_output, const at::Tensor & weight) {
    return at::_ops::mkldnn_linear_backward_input::call(input_size, grad_output, weight);
}

// aten::mkldnn_linear_backward_input.out(int[] input_size, Tensor grad_output, Tensor weight, *, Tensor(a!) out) -> Tensor(a!)
inline at::Tensor & mkldnn_linear_backward_input_out(at::Tensor & out, at::IntArrayRef input_size, const at::Tensor & grad_output, const at::Tensor & weight) {
    return at::_ops::mkldnn_linear_backward_input_out::call(input_size, grad_output, weight, out);
}
// aten::mkldnn_linear_backward_input.out(int[] input_size, Tensor grad_output, Tensor weight, *, Tensor(a!) out) -> Tensor(a!)
inline at::Tensor & mkldnn_linear_backward_input_outf(at::IntArrayRef input_size, const at::Tensor & grad_output, const at::Tensor & weight, at::Tensor & out) {
    return at::_ops::mkldnn_linear_backward_input_out::call(input_size, grad_output, weight, out);
}

}
