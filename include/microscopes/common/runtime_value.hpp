#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/assert.hpp>

namespace microscopes {
namespace common {

class value_accessor {
public:
  value_accessor() : data_(), mask_(), type_() {}
  template <typename T>
  value_accessor(const T *data)
    : data_(reinterpret_cast<const uint8_t *>(data)),
      mask_(nullptr),
      type_(runtime_type(static_type_to_primitive_type<T>::value)) {}
  value_accessor(const uint8_t *data,
                 const bool *mask,
                 const runtime_type &type)
    : data_(data), mask_(mask), type_(type) {}

  inline const runtime_type & type() const { return type_; }
  inline unsigned shape() const { return type_.n(); }

  inline bool
  ismasked(size_t idx) const
  {
    MICROSCOPES_ASSERT(idx < shape());
    return !mask_ ? false : mask_[idx];
  }

  inline bool
  anymasked() const
  {
    if (!mask_)
      return false;
    // XXX: more efficient ways to do this!
    for (size_t i = 0; i < shape(); i++)
      if (ismasked(i))
        return true;
    return false;
  }

  template <typename T>
  inline T
  get(size_t idx) const
  {
    MICROSCOPES_ASSERT(data_);
    MICROSCOPES_ASSERT(idx < shape());
    const size_t s = runtime_type_traits::PrimitiveTypeSize(type_.t());
    return runtime_cast::cast<T>(data_ + idx * s, type_.t());
  }

  std::string debug_str() const;

private:
  const uint8_t *data_;
  const bool *mask_;
  runtime_type type_;
};

class value_mutator {
public:
  value_mutator() : data_(), type_() {}
  value_mutator(uint8_t *data, const runtime_type &type)
    : data_(data), type_(type) {}

  inline const runtime_type & type() const { return type_; }
  inline unsigned shape() const { return type_.n(); }

  template <typename T>
  inline void
  set(T t, size_t idx)
  {
    MICROSCOPES_ASSERT(data_);
    MICROSCOPES_ASSERT(idx < shape());
    const size_t s = runtime_type_traits::PrimitiveTypeSize(type_.t());
    runtime_cast::uncast<T>(data_ + idx * s, type_.t(), t);
  }

  inline value_accessor
  accessor() const
  {
    return value_accessor(data_, nullptr, type_);
  }

private:
  uint8_t *data_;
  runtime_type type_;
};

} // namespace common
} // namespace microscopes
