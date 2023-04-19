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

TORCH_API at::Tensor randint(int64_t high, at::IntArrayRef size, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint(int64_t high, at::IntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor randint_symint(int64_t high, c10::SymIntArrayRef size, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint_symint(int64_t high, c10::SymIntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & randint_out(at::Tensor & out, int64_t high, at::IntArrayRef size);
TORCH_API at::Tensor & randint_outf(int64_t high, at::IntArrayRef size, at::Tensor & out);
TORCH_API at::Tensor & randint_symint_out(at::Tensor & out, int64_t high, c10::SymIntArrayRef size);
TORCH_API at::Tensor & randint_symint_outf(int64_t high, c10::SymIntArrayRef size, at::Tensor & out);
TORCH_API at::Tensor randint(int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint(int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor randint_symint(int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint_symint(int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & randint_out(at::Tensor & out, int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator);
TORCH_API at::Tensor & randint_outf(int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator, at::Tensor & out);
TORCH_API at::Tensor & randint_symint_out(at::Tensor & out, int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator);
TORCH_API at::Tensor & randint_symint_outf(int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator, at::Tensor & out);
TORCH_API at::Tensor randint(int64_t low, int64_t high, at::IntArrayRef size, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint(int64_t low, int64_t high, at::IntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor randint_symint(int64_t low, int64_t high, c10::SymIntArrayRef size, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint_symint(int64_t low, int64_t high, c10::SymIntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & randint_out(at::Tensor & out, int64_t low, int64_t high, at::IntArrayRef size);
TORCH_API at::Tensor & randint_outf(int64_t low, int64_t high, at::IntArrayRef size, at::Tensor & out);
TORCH_API at::Tensor & randint_symint_out(at::Tensor & out, int64_t low, int64_t high, c10::SymIntArrayRef size);
TORCH_API at::Tensor & randint_symint_outf(int64_t low, int64_t high, c10::SymIntArrayRef size, at::Tensor & out);
TORCH_API at::Tensor randint(int64_t low, int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint(int64_t low, int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor randint_symint(int64_t low, int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator, at::TensorOptions options=at::kLong);
TORCH_API at::Tensor randint_symint(int64_t low, int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory);
TORCH_API at::Tensor & randint_out(at::Tensor & out, int64_t low, int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator);
TORCH_API at::Tensor & randint_outf(int64_t low, int64_t high, at::IntArrayRef size, c10::optional<at::Generator> generator, at::Tensor & out);
TORCH_API at::Tensor & randint_symint_out(at::Tensor & out, int64_t low, int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator);
TORCH_API at::Tensor & randint_symint_outf(int64_t low, int64_t high, c10::SymIntArrayRef size, c10::optional<at::Generator> generator, at::Tensor & out);

} // namespace compositeexplicitautograd
} // namespace at
