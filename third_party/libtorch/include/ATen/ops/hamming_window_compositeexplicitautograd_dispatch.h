#pragma once
// @generated by torchgen/gen.py from DispatchKeyFunction.h

// NB: The implementing C++ file is RegisterDispatchKey.cpp

// The only #includes we need are for custom classes that have defaults in the C++ API
#include <c10/core/MemoryFormat.h>
#include <c10/core/Scalar.h>
#include <ATen/core/Reduction.h>

// Forward declarations of any types needed in the operator signatures.
// We can't directly include these classes because it will cause circular include dependencies.
// This file is included by TensorBody.h, which defines the Tensor class.
#include <ATen/core/ATen_fwd.h>

namespace at {

namespace compositeexplicitautograd {

TORCH_API at::Tensor hamming_window(int64_t window_length, at::TensorOptions options={});
TORCH_API at::Tensor hamming_window(int64_t window_length, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & hamming_window_out(at::Tensor & out, int64_t window_length);
TORCH_API at::Tensor & hamming_window_outf(int64_t window_length, at::Tensor & out);
TORCH_API at::Tensor hamming_window(int64_t window_length, bool periodic, at::TensorOptions options={});
TORCH_API at::Tensor hamming_window(int64_t window_length, bool periodic, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & hamming_window_out(at::Tensor & out, int64_t window_length, bool periodic);
TORCH_API at::Tensor & hamming_window_outf(int64_t window_length, bool periodic, at::Tensor & out);
TORCH_API at::Tensor hamming_window(int64_t window_length, bool periodic, double alpha, at::TensorOptions options={});
TORCH_API at::Tensor hamming_window(int64_t window_length, bool periodic, double alpha, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & hamming_window_out(at::Tensor & out, int64_t window_length, bool periodic, double alpha);
TORCH_API at::Tensor & hamming_window_outf(int64_t window_length, bool periodic, double alpha, at::Tensor & out);
TORCH_API at::Tensor hamming_window(int64_t window_length, bool periodic, double alpha, double beta, at::TensorOptions options={});
TORCH_API at::Tensor hamming_window(int64_t window_length, bool periodic, double alpha, double beta, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & hamming_window_out(at::Tensor & out, int64_t window_length, bool periodic, double alpha, double beta);
TORCH_API at::Tensor & hamming_window_outf(int64_t window_length, bool periodic, double alpha, double beta, at::Tensor & out);

} // namespace compositeexplicitautograd
} // namespace at
