#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/random_fwd.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/assert.hpp>

#include <vector>
#include <cstring>
#include <iostream>

namespace microscopes {
namespace common {
namespace recarray {

class row_accessor {
  friend class row_mutator;
public:
  row_accessor()
    : data_(), mask_(), types_(),
      cursor_(), mask_cursor_(), pos_() {}
  row_accessor(const uint8_t *data,
               const bool *mask,
               const std::vector<runtime_type> *types)
    : data_(data), mask_(mask), types_(types),
      cursor_(data), mask_cursor_(mask), pos_()
  {
    MICROSCOPES_ASSERT(data);
    MICROSCOPES_ASSERT(types);
  }

  inline size_t tell() const { return pos_; }
  inline size_t nfeatures() const { return types_->size(); }

  inline const runtime_type & curtype() const { return (*types_)[pos_]; }
  inline unsigned curshape() const { return curtype().n(); }

  inline value_accessor get() const { return value_accessor(cursor_, mask_cursor_, curtype()); }

  inline bool
  ismasked(size_t idx) const
  {
    MICROSCOPES_ASSERT(idx < curshape());
    return get().ismasked(idx);
  }

  inline bool
  anymasked() const
  {
    return get().anymasked();
  }

  template <typename T>
  inline T
  get(size_t idx) const
  {
    MICROSCOPES_ASSERT(pos_ < nfeatures());
    get().get<T>(idx);
  }

  inline void
  bump()
  {
    MICROSCOPES_ASSERT(pos_ <= nfeatures());
    cursor_ += runtime_type_traits::RuntimeTypeSize(curtype());
    if (mask_)
      mask_cursor_ += curtype().n();
    pos_++;
  }

  inline bool end() const { return pos_ == nfeatures(); }

  inline void
  reset()
  {
    cursor_ = data_;
    mask_cursor_ = mask_;
    pos_ = 0;
  }

  std::string debug_str() const;

protected:
  inline const uint8_t * cursor() const { return cursor_; }

private:
  const uint8_t *data_;
  const bool *mask_;
  const std::vector<runtime_type> *types_;

  const uint8_t *cursor_;
  const bool *mask_cursor_;
  size_t pos_;
};

class row_mutator {
public:
  row_mutator()
    : data_(), types_(), cursor_(), pos_() {}
  row_mutator(uint8_t *data,
              const std::vector<runtime_type> *types)
    : data_(data), types_(types),
      cursor_(data), pos_()
  {
    MICROSCOPES_ASSERT(data);
    MICROSCOPES_ASSERT(types);
  }

  inline size_t tell() const { return pos_; }
  inline size_t nfeatures() const { return types_->size(); }

  inline const runtime_type & curtype() const { return (*types_)[pos_]; }
  inline unsigned curshape() const { return curtype().n(); }

  inline value_mutator set() const { return value_mutator(cursor_, curtype()); }

  template <typename T>
  inline void
  set(T t, size_t idx)
  {
    MICROSCOPES_ASSERT(pos_ < nfeatures());
    set().set<T>(t, idx);
  }

  void
  set(const row_accessor &acc)
  {
    MICROSCOPES_DCHECK(curshape() == acc.curshape(), "shapes do not match");
    MICROSCOPES_ASSERT(cursor_);
    MICROSCOPES_ASSERT(acc.cursor());
    const size_t s0 = runtime_type_traits::PrimitiveTypeSize(curtype().t());
    const size_t s1 = runtime_type_traits::PrimitiveTypeSize(acc.curtype().t());
    for (unsigned i = 0; i < curshape(); i++)
      runtime_cast::copy(
          cursor_ + i * s0, curtype().t(),
          acc.cursor() + i * s1, acc.curtype().t());
  }

  inline void
  bump()
  {
    MICROSCOPES_ASSERT(pos_ <= nfeatures());
    cursor_ += runtime_type_traits::RuntimeTypeSize(curtype());
    pos_++;
  }

  inline bool end() const { return pos_ == nfeatures(); }

  inline void
  reset()
  {
    cursor_ = data_;
    pos_ = 0;
  }

  std::string debug_str() const;

private:
  uint8_t *data_;
  const std::vector<runtime_type> *types_;

  uint8_t *cursor_;
  size_t pos_;
};

class dataview {
protected:
  dataview(size_t n, const std::vector<runtime_type> &types);

  inline const std::vector<size_t> & offsets() const { return offsets_; }

  // in bytes
  inline size_t rowsize() const { return rowsize_; }
  inline size_t maskrowsize() const { return maskrowsize_; }

public:
  virtual ~dataview() {}

  // implementations need to provide the following API
  virtual row_accessor get() const = 0;
  virtual size_t index() const = 0;
  virtual void next() = 0;
  virtual void reset() = 0;
  virtual bool end() const = 0;

  inline size_t size() const { return n_; }
  inline const std::vector<runtime_type> & types() const { return types_; }

private:
  size_t n_;

  std::vector<runtime_type> types_;
  std::vector<size_t> offsets_;
  size_t rowsize_;
  size_t maskrowsize_;
};

class row_major_dataview : public dataview {
public:
  row_major_dataview(const uint8_t *data,
                     const bool *mask,
                     size_t n,
                     const std::vector<runtime_type> &types);
  row_accessor get() const override;
  size_t index() const override;
  void next() override;
  void reset() override;
  bool end() const override;

  inline void reset_permutation() { pi_.clear(); }
  void permute(rng_t &rng);

private:
  const uint8_t *data_;
  const bool *mask_;
  size_t pos_;

  std::vector<size_t> pi_;
};

} // namespace recarray
} // namespace common
} // namespace microscopes
