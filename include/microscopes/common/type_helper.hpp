#pragma once

#include <microscopes/common/type_info.h>
#include <microscopes/common/macros.hpp>

#include <vector>
#include <utility>
#include <iostream>
#include <sstream>

// XXX: handle unsigned types

#define PRIMITIVE_TYPE_MAPPINGS(x) \
  x(bool, TYPE_INFO_B) \
  x(int8_t, TYPE_INFO_I8) \
  x(uint8_t, TYPE_INFO_I8) \
  x(int16_t, TYPE_INFO_I16) \
  x(uint16_t, TYPE_INFO_I16) \
  x(int32_t, TYPE_INFO_I32) \
  x(uint32_t, TYPE_INFO_I32) \
  x(int64_t, TYPE_INFO_I64) \
  x(uint64_t, TYPE_INFO_I64) \
  x(float, TYPE_INFO_F32) \
  x(double, TYPE_INFO_F64)

#define CANONICAL_PRIMITIVE_TYPE_MAPPINGS(x) \
  x(bool, TYPE_INFO_B) \
  x(int8_t, TYPE_INFO_I8) \
  x(int16_t, TYPE_INFO_I16) \
  x(int32_t, TYPE_INFO_I32) \
  x(int64_t, TYPE_INFO_I64) \
  x(float, TYPE_INFO_F32) \
  x(double, TYPE_INFO_F64)

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
PRIMITIVE_TYPE_MAPPINGS(SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID)
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

  static const char *
  TypeInfoString(runtime_type_info t)
  {
#define _STRING_CASE(x) \
    if (t == x) \
      return #x;
    RUNTIME_TYPE_INFO(_STRING_CASE)
#undef _STRING_CASE
    MICROSCOPES_NOT_REACHABLE();
    return nullptr;
  }

  static std::string
  ToString(runtime_type_info t, const uint8_t *px)
  {
    std::ostringstream oss;
    switch (t) {
#define _CASE_STMT(ctype, rtype) \
      case rtype: \
        oss << *reinterpret_cast< const ctype * >(px); \
        break;
    CANONICAL_PRIMITIVE_TYPE_MAPPINGS(_CASE_STMT)
#undef _CASE_STMT
    default:
      MICROSCOPES_NOT_REACHABLE();
      break;
    }
    return oss.str();
  }

private:
  static const size_t TypeSizes_[TYPE_INFO_NELEMS];
};

template <typename T>
struct runtime_cast {

  static inline ALWAYS_INLINE T
  cast(const uint8_t *px, runtime_type_info t)
  {
    // XXX: branch predictor hints
    switch (t) {
#define _CASE_STMT(ctype, rtype) \
      case rtype: \
        return *reinterpret_cast< const ctype * >(px);
    CANONICAL_PRIMITIVE_TYPE_MAPPINGS(_CASE_STMT)
#undef _CASE_STMT
    default:
      break;
    }
    MICROSCOPES_NOT_REACHABLE();
    return T();
  }

  static inline ALWAYS_INLINE void
  uncast(uint8_t *px, runtime_type_info t, T value)
  {
    // XXX: branch predictor hints
    switch (t) {
#define _CASE_STMT(ctype, rtype) \
      case rtype: \
        *reinterpret_cast< ctype *>(px) = value; \
        return;
    CANONICAL_PRIMITIVE_TYPE_MAPPINGS(_CASE_STMT)
#undef _CASE_STMT
    default:
      break;
    }
    MICROSCOPES_NOT_REACHABLE();
  }
};

} // namespace common
} // namespace microscopes
