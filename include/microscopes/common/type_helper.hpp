#pragma once

#include <microscopes/common/type_info.h>

#include <vector>
#include <utility>
#include <iostream>

namespace microscopes {
namespace common {

template <typename T>
struct _info {
  static const size_t size = sizeof(T);
};

template <typename T>
struct _static_type_to_runtime_id {};

#define SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(tpe, rvalue) \
  template <> struct _static_type_to_runtime_id <tpe> { static const runtime_type_info value = rvalue; };

// XXX: handle unsigned types differently
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(bool, TYPE_INFO_B)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(int8_t, TYPE_INFO_I8)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(uint8_t, TYPE_INFO_I8)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(int16_t, TYPE_INFO_I16)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(uint16_t, TYPE_INFO_I16)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(int32_t, TYPE_INFO_I32)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(uint32_t, TYPE_INFO_I32)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(int64_t, TYPE_INFO_I64)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(uint64_t, TYPE_INFO_I64)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(float, TYPE_INFO_F32)
SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(double, TYPE_INFO_F64)

#undef SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID

class runtime_type_traits {
public:
  static inline size_t
  TypeSize(runtime_type_info t)
  {
    return TypeSizes_[t];
  }

  static inline std::pair< std::vector<size_t>, size_t >
  GetOffsetsAndSize(const std::vector<runtime_type_info> &types)
  {
    std::vector<size_t> offsets;
    offsets.reserve(types.size());
    size_t acc = 0;
    for (auto t : types) {
      offsets.push_back(acc);
      acc += TypeSize(t);
    }
    return std::make_pair( std::move(offsets), acc );
  }

private:
  static const size_t TypeSizes_[TYPE_INFO_NELEMS];
};

} // namespace common
} // namespace microscopes
