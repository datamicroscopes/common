#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/assert.hpp>

#include <vector>
#include <cstring>
#include <iostream>

namespace microscopes {
namespace common {
namespace variadic {

class row_accessor {
public:
  row_accessor() : data_(), type_() {}
  row_accessor(const uint8_t *data,
               const runtime_type *type,
               size_t n)
    : data_(data), type_(type), n_(n)
  {
    MICROSCOPES_ASSERT(data);
    MICROSCOPES_ASSERT(type);
  }

  inline size_t n() const { return n_; }
  inline const runtime_type & type() const { return *type_; }

  inline value_accessor
  get(size_t idx) const
  {
    MICROSCOPES_DCHECK(idx < n() , "invalid idx");
    const size_t off = idx * type().size();
    return value_accessor(data_ + off, nullptr, type());
  }

  std::string debug_str() const;

private:
  const uint8_t *data_;
  const runtime_type *type_;
  size_t n_;
};

// XXX(stephentu): row_mutator

class dataview {
protected:
  dataview(size_t n, const runtime_type &type)
    : n_(n), type_(type) {}

public:
  virtual ~dataview() {}

  // implementations need to provide the following API
  virtual row_accessor get(size_t i) const = 0;
  virtual size_t rowsize(size_t i) const = 0;

  inline size_t size() const { return n_; }
  inline const runtime_type & type() const { return type_; }

private:
  size_t n_;
  runtime_type type_;
};

class row_major_dataview : public dataview {
public:
  row_major_dataview(const uint8_t *data,
                     const std::vector<unsigned> &ns,
                     const runtime_type &type)
    : dataview(ns.size(), type), ns_(ns)
  {
    MICROSCOPES_ASSERT(data);
    pxs_.reserve(ns.size());
    const uint8_t *px = data;
    for (auto n : ns) {
      pxs_.push_back(px);
      px += type.size() * n;
    }
  }

  row_accessor
  get(size_t i) const override
  {
    MICROSCOPES_DCHECK(i < size(), "invalid i");
    return row_accessor(pxs_[i], &type(), ns_[i]);
  }

  size_t
  rowsize(size_t i) const override
  {
    MICROSCOPES_DCHECK(i < size(), "invalid i");
    return ns_[i];
  }

private:
  std::vector<const uint8_t *> pxs_;
  std::vector<unsigned> ns_;
};

} // namespace variadic
} // namespace common
} // namespace microscopes
