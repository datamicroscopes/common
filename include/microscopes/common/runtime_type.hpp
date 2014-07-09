#pragma once

#include <microscopes/common/type_info.h>
#include <microscopes/common/assert.hpp>
#include <microscopes/common/macros.hpp>

#include <vector>
#include <utility>
#include <iostream>
#include <sstream>

namespace microscopes {
namespace common {

template <typename T>
struct static_type_to_primitive_type {};

#define SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID(tpe, rvalue) \
  template <> struct static_type_to_primitive_type <tpe> { static const primitive_type value = rvalue; };
PRIMITIVE_TYPE_MAPPINGS(SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID)
#undef SPECIALIZE_STATIC_TYPE_TO_RUNTIME_ID

class runtime_type {
public:
  // default ctor
  runtime_type() : t_(), n_(), vec_() {}

  // scalar constructor
  runtime_type(primitive_type t) : t_(t), n_(1), vec_(false) {}

  // vector constructor
  runtime_type(primitive_type t, unsigned n)
    : t_(t), n_(n), vec_(true)
  {
  }

  inline bool
  operator==(const runtime_type &that) const
  {
    return t_ == that.t_ && n_ == that.n_ && vec_ == that.vec_;
  }

  inline bool
  operator!=(const runtime_type &that) const
  {
    return !operator==(that);
  }

  inline primitive_type t() const { return t_; }
  inline unsigned n() const { return n_; }
  inline bool vec() const { return vec_; }

private:
  primitive_type t_;
  unsigned n_;
  bool vec_;
};

class runtime_type_traits {
public:

  static inline size_t
  PrimitiveTypeSize(primitive_type t)
  {
    MICROSCOPES_ASSERT(t < TYPE_NELEMS);
    return PrimitiveTypeSizes_[t];
  }

  static inline size_t
  RuntimeTypeSize(const runtime_type &t)
  {
    return t.n() * PrimitiveTypeSize(t.t());
  }

  struct offsets_ret_t {
    offsets_ret_t() : offsets_(), rowsize_(), maskrowsize_() {}
    std::vector<size_t> offsets_;
    size_t rowsize_;
    size_t maskrowsize_;
  };

  static inline offsets_ret_t
  GetOffsetsAndSize(const std::vector<runtime_type> &types)
  {
    offsets_ret_t ret;
    ret.offsets_.reserve(types.size());
    for (const auto &t : types) {
      ret.offsets_.push_back(ret.rowsize_);
      ret.rowsize_ += RuntimeTypeSize(t);
      ret.maskrowsize_ += t.n();
    }
    return ret;
  }

  static const char *
  PrimitiveTypeStr(primitive_type t)
  {
#define _STRING_CASE(x) if (t == x) return #x;
    PRIMITIVE_TYPES(_STRING_CASE)
#undef _STRING_CASE
    MICROSCOPES_NOT_REACHABLE();
    return nullptr;
  }

  static std::string
  RuntimeTypeStr(const runtime_type &t)
  {
    std::ostringstream oss;
    oss << PrimitiveTypeStr(t.t());
    if (t.vec())
      oss << "[" << t.n() << "]";
    return oss.str();
  }

  static std::string
  ToString(primitive_type t, const uint8_t *px)
  {
    std::ostringstream oss;
    switch (t) {
#define _CASE_STMT(ctype, rtype) \
      case rtype: \
        oss << *reinterpret_cast< const ctype * >(px); \
        break;
    PRIMITIVE_TYPE_MAPPINGS(_CASE_STMT)
#undef _CASE_STMT
    default:
      MICROSCOPES_NOT_REACHABLE();
      break;
    }
    return oss.str();
  }

private:
  static const size_t PrimitiveTypeSizes_[TYPE_NELEMS];
};

struct runtime_cast {

  template <typename T>
  static inline ALWAYS_INLINE T
  cast(const uint8_t *px, primitive_type t)
  {
    switch (t) {
#define _CASE_STMT(ctype, rtype) \
      case rtype: \
        return *reinterpret_cast< const ctype * >(px);
    PRIMITIVE_TYPE_MAPPINGS(_CASE_STMT)
#undef _CASE_STMT
    default:
      break;
    }
    MICROSCOPES_NOT_REACHABLE();
    return T();
  }

  template <typename T>
  static inline ALWAYS_INLINE void
  uncast(uint8_t *px, primitive_type t, T value)
  {
    switch (t) {
#define _CASE_STMT(ctype, rtype) \
      case rtype: \
        *reinterpret_cast< ctype *>(px) = value; \
        return;
    PRIMITIVE_TYPE_MAPPINGS(_CASE_STMT)
#undef _CASE_STMT
    default:
      break;
    }
    MICROSCOPES_NOT_REACHABLE();
  }

  static inline void
  copy(uint8_t *dst, primitive_type dst_t,
       const uint8_t *src, primitive_type src_t)
  {
    switch (src_t) {
#define _SRC_CASE_STMT(src_ctype, src_rtype) \
      case src_rtype: \
        uncast(dst, dst_t, *reinterpret_cast<const src_ctype *>(src)); \
        return;
    PRIMITIVE_TYPE_MAPPINGS(_SRC_CASE_STMT)
#undef _SRC_CASE_STMT
    default:
      MICROSCOPES_NOT_REACHABLE();
      break;
    }
  }
};

} // namespace common
} // namespace microscopes
