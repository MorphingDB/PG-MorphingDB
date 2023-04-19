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



#include <ATen/ops/from_file_ops.h>

namespace at {


// aten::from_file(str filename, bool? shared=None, int? size=0, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None) -> Tensor
inline at::Tensor from_file(c10::string_view filename, c10::optional<bool> shared=c10::nullopt, c10::optional<int64_t> size=0, at::TensorOptions options={}) {
    return at::_ops::from_file::call(filename, shared, size, optTypeMetaToScalarType(options.dtype_opt()), options.layout_opt(), options.device_opt(), options.pinned_memory_opt());
}
// aten::from_file(str filename, bool? shared=None, int? size=0, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None) -> Tensor
inline at::Tensor from_file(c10::string_view filename, c10::optional<bool> shared, c10::optional<int64_t> size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory) {
    return at::_ops::from_file::call(filename, shared, size, dtype, layout, device, pin_memory);
}

// aten::from_file.out(str filename, bool? shared=None, int? size=0, *, Tensor(a!) out) -> Tensor(a!)
inline at::Tensor & from_file_out(at::Tensor & out, c10::string_view filename, c10::optional<bool> shared=c10::nullopt, c10::optional<int64_t> size=0) {
    return at::_ops::from_file_out::call(filename, shared, size, out);
}
// aten::from_file.out(str filename, bool? shared=None, int? size=0, *, Tensor(a!) out) -> Tensor(a!)
inline at::Tensor & from_file_outf(c10::string_view filename, c10::optional<bool> shared, c10::optional<int64_t> size, at::Tensor & out) {
    return at::_ops::from_file_out::call(filename, shared, size, out);
}

}
