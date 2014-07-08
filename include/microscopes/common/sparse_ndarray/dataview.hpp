#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/macros.hpp>

#include <algorithm>

namespace microscopes {
namespace common {
namespace sparse_ndarray {

class dataview {
public:

  dataview(const std::vector<size_t> &shape, const runtime_type &type)
    : shape_(shape), type_(type)
  {
    // assume no zero-dimesional things
    MICROSCOPES_DCHECK(shape_.size(), "zero-d array not allowed");
    for (auto s : shape)
      MICROSCOPES_DCHECK(s, "empty-dimesion not allowed");
  }

  inline size_t dims() const { return shape_.size(); }
  inline const std::vector<size_t> & shape() const { return shape_; }
  inline const runtime_type & type() const { return t_; }

  virtual value_accessor get(const std::vector<size_t> &indices) const = 0;

protected:
  std::vector<size_t> shape_;
  runtime_type type_;
};

/**
 * useful for testing
 */
class row_major_dense_dataview : public dataview {
public:
  row_major_dense_dataview(const uint8_t *data,
                           const bool *mask,
                           const std::vector<size_t> &shape,
                           const runtime_type &type)
    : dataview(shape, type), data_(data), mask_(mask),
      stepsize_(runtime_type_traits::RuntimeTypeSize(type))
  {
    MICROSCOPES_DCHECK(data, "data cannot be null");
    multipliers_.push_back(1);
    auto rit = shape.rbegin();
    for (size_t i = 0; i < shape.size() - 1; ++i, ++rit)
      multipliers_.push_back(multipliers_.back() * (*rit));
    std::reverse(multipliers_.begin(), multipliers_.end());
  }

  value_accessor
  get(const std::vector<size_t> &indices) const override
  {
    MICROSCOPES_DCHECK(dims() == indices.size(), "invalid # of indices");
    for (size_t i = 0; i < dims(); i++)
      MICROSCOPES_DCHECK(indices_[i] < shape_[i], "index out of bounds");

    size_t off = 0;
    for (size_t i = 0; i < dims(); i++)
      off += indices[i] * multipliers_[i];

    return value_accessor(
        data_ + stepsize_ * off,
        mask_ ? (mask_ + off) : nullptr,
        type());
  }

private:
  const uint8_t *data_;
  const bool *mask_;
  size_t stepsize_;
  std::vector<size_t> multipliers_;
};

} // namespace sparse_ndarray
} // namespace common
} // namespace microscopes
