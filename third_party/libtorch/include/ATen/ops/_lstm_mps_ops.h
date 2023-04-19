#pragma once

// @generated by torchgen/gen.py from Operator.h

#include <tuple>
#include <vector>

// Forward declarations of any types needed in the operator signatures.
// We can't directly include these classes because it will cause circular include dependencies.
// This file is included by TensorBody.h, which defines the Tensor class.
#include <ATen/core/ATen_fwd.h>

namespace at {
namespace _ops {


struct TORCH_API _lstm_mps {
  using schema = ::std::tuple<at::Tensor,at::Tensor,at::Tensor,at::Tensor,at::Tensor,at::Tensor> (const at::Tensor &, at::TensorList, at::TensorList, bool, int64_t, double, bool, bool, bool);
  using ptr_schema = schema*;
  // See Note [static constexpr char* members for windows NVCC]
  STATIC_CONSTEXPR_STR_INL_EXCEPT_WIN_CUDA(name, "aten::_lstm_mps")
  STATIC_CONSTEXPR_STR_INL_EXCEPT_WIN_CUDA(overload_name, "")
  STATIC_CONSTEXPR_STR_INL_EXCEPT_WIN_CUDA(schema_str, "_lstm_mps(Tensor input, Tensor[] hx, Tensor[] params, bool has_biases, int num_layers, float dropout, bool train, bool bidirectional, bool batch_first) -> (Tensor, Tensor, Tensor, Tensor, Tensor, Tensor)")
  static ::std::tuple<at::Tensor,at::Tensor,at::Tensor,at::Tensor,at::Tensor,at::Tensor> call(const at::Tensor & input, at::TensorList hx, at::TensorList params, bool has_biases, int64_t num_layers, double dropout, bool train, bool bidirectional, bool batch_first);
  static ::std::tuple<at::Tensor,at::Tensor,at::Tensor,at::Tensor,at::Tensor,at::Tensor> redispatch(c10::DispatchKeySet dispatchKeySet, const at::Tensor & input, at::TensorList hx, at::TensorList params, bool has_biases, int64_t num_layers, double dropout, bool train, bool bidirectional, bool batch_first);
};

struct TORCH_API _lstm_mps_out {
  using schema = ::std::tuple<at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &> (const at::Tensor &, at::TensorList, at::TensorList, bool, int64_t, double, bool, bool, bool, at::Tensor &, at::Tensor &, at::Tensor &, at::Tensor &, at::Tensor &, at::Tensor &);
  using ptr_schema = schema*;
  // See Note [static constexpr char* members for windows NVCC]
  STATIC_CONSTEXPR_STR_INL_EXCEPT_WIN_CUDA(name, "aten::_lstm_mps")
  STATIC_CONSTEXPR_STR_INL_EXCEPT_WIN_CUDA(overload_name, "out")
  STATIC_CONSTEXPR_STR_INL_EXCEPT_WIN_CUDA(schema_str, "_lstm_mps.out(Tensor input, Tensor[] hx, Tensor[] params, bool has_biases, int num_layers, float dropout, bool train, bool bidirectional, bool batch_first, *, Tensor(a!) out0, Tensor(b!) out1, Tensor(c!) out2, Tensor(d!) out3, Tensor(e!) out4, Tensor(f!) out5) -> (Tensor(a!), Tensor(b!), Tensor(c!), Tensor(d!), Tensor(e!), Tensor(f!))")
  static ::std::tuple<at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &> call(const at::Tensor & input, at::TensorList hx, at::TensorList params, bool has_biases, int64_t num_layers, double dropout, bool train, bool bidirectional, bool batch_first, at::Tensor & out0, at::Tensor & out1, at::Tensor & out2, at::Tensor & out3, at::Tensor & out4, at::Tensor & out5);
  static ::std::tuple<at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &,at::Tensor &> redispatch(c10::DispatchKeySet dispatchKeySet, const at::Tensor & input, at::TensorList hx, at::TensorList params, bool has_biases, int64_t num_layers, double dropout, bool train, bool bidirectional, bool batch_first, at::Tensor & out0, at::Tensor & out1, at::Tensor & out2, at::Tensor & out3, at::Tensor & out4, at::Tensor & out5);
};

}} // namespace at::_ops
