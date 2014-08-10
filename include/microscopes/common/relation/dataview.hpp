#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/util.hpp>

#include <algorithm>
#include <iterator>
#include <memory>

namespace microscopes {
namespace common {
namespace relation {

/**
 * A relation dataview only supports two operations:
 *   (A) retrieving a value at a given position: get(indices)
 *   (B) slice along one particular dimension: slice(dim)
 *
 * Implementations should do their best to ensure that (B) is as fast as
 * possible.  (A) should be correct, but is not intended to be in the critical
 * path
 */
class dataview {
public:

  typedef std::pair<std::vector<size_t>, value_accessor> value_with_position_t;

  // subclasses need to provide an implementation of a slice iterator
  //
  // XXX(stephentu): this interface sucks
  //   (A) causes implementations to be needlessly inefficient, and
  //   (B) it feels too java like
  class slice_iterator_impl {
  public:
    virtual ~slice_iterator_impl() {}
    virtual std::unique_ptr<slice_iterator_impl> clone() const = 0;
    virtual bool equals(const slice_iterator_impl &) const = 0;
    virtual void next() = 0;
    virtual const value_with_position_t & value() const = 0;
  };

  class slice_iterator :
    public std::iterator<std::input_iterator_tag, const value_with_position_t>
  {
  public:
    slice_iterator() : impl_() {}
    slice_iterator(std::unique_ptr<slice_iterator_impl> &&impl)
      : impl_(std::move(impl)) {}
    slice_iterator(const slice_iterator &that)
      : impl_(that.impl_->clone()) {}
    slice_iterator(slice_iterator &&that)
      : impl_(std::move(that.impl_)) {}

    inline slice_iterator &
    operator=(const slice_iterator &that)
    {
      if (!that.impl_)
        impl_.reset();
      else
        impl_ = std::move(that.impl_->clone());
      return *this;
    }

    inline bool
    operator==(const slice_iterator &that) const
    {
      if (!impl_)
        return !that.impl_;
      else if (!that.impl_)
        return !impl_;
      else
        return impl_->equals(*that.impl_);
    }

    inline bool
    operator!=(const slice_iterator &that) const
    {
      return !operator==(that);
    }

    inline slice_iterator &
    operator++()
    {
      impl_->next();
      return *this;
    }

    inline slice_iterator
    operator++(int)
    {
      slice_iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    inline const value_with_position_t &
    operator*() const
    {
      return impl_->value();
    }

    inline const value_with_position_t *
    operator->() const
    {
      return &impl_->value();
    }

  private:
    std::unique_ptr<slice_iterator_impl> impl_;
  };

  class slice_iterable {
  public:
    slice_iterable() : begin_(), end_() {}
    slice_iterable(const slice_iterator &begin,
                   const slice_iterator &end)
      : begin_(begin),
        end_(end) {}
    slice_iterable(slice_iterator &&begin,
                   slice_iterator &&end)
      : begin_(std::move(begin)),
        end_(std::move(end)) {}

    inline slice_iterator begin() const { return begin_; }
    inline slice_iterator end() const { return end_; }

  private:
    slice_iterator begin_;
    slice_iterator end_;
  };

  dataview(const std::vector<size_t> &shape, const runtime_type &type)
    : shape_(shape), type_(type)
  {
    // assume no zero-dimesional things
    MICROSCOPES_DCHECK(shape_.size(), "zero-d array not allowed");
#ifdef DEBUG_MODE
    for (auto s : shape)
      MICROSCOPES_DCHECK(s, "empty-dimesion not allowed");
#endif
  }

  virtual ~dataview() {}

  inline size_t dims() const { return shape_.size(); }
  inline const std::vector<size_t> & shape() const { return shape_; }
  inline const runtime_type & type() const { return type_; }

  // the API which subclasses need to provide

  virtual value_accessor get(const std::vector<size_t> &indices) const = 0;
  virtual slice_iterable slice(size_t dim, size_t idx) const = 0;

protected:
  std::vector<size_t> shape_;
  runtime_type type_;
};

namespace detail {

class product {
public:

  enum tag_t { SINGLETON, RANGE, };

  struct indices {
    indices() : tag_(), v_() {}
    indices(tag_t tag, size_t v)
      : tag_(tag), v_(v) {}
    tag_t tag_;
    size_t v_;
  };

  product(const std::vector<indices> &is)
    : is_(is)
  {
    cur_.resize(is.size());
    reset();
  }

  inline void
  reset()
  {
    end_ = true;
    pos_ = 0;
    for (size_t i = 0; i < is_.size(); i++) {
      end_ = false;
      if (is_[i].tag_ == SINGLETON) {
        cur_[i] = is_[i].v_;
      } else {
        MICROSCOPES_ASSERT(is_[i].tag_ == RANGE);
        MICROSCOPES_ASSERT(is_[i].v_);
        cur_[i] = 0;
      }
    }
  }

  inline void
  setEnd()
  {
    end_ = true;
    pos_ = 1;
    for (size_t i = 0; i < is_.size(); i++) {
      if (is_[i].tag_ == SINGLETON) {
        cur_[i] = is_[i].v_;
      } else {
        cur_[i] = 0;
        pos_ *= is_[i].v_;
      }
    }
  }

  inline bool
  end() const
  {
    return end_;
  }

  inline size_t
  pos() const
  {
    return pos_;
  }

  inline const std::vector<size_t> &
  value() const
  {
    MICROSCOPES_ASSERT(!end());
    return cur_;
  }

  inline void
  next()
  {
    MICROSCOPES_ASSERT(!end_);
    for (ssize_t i = is_.size() - 1; i >= 0; i--) {
      if (is_[i].tag_ == SINGLETON)
        continue;
      if ((cur_[i]+1) < is_[i].v_) {
        cur_[i]++;
        pos_++;
        return;
      }
      cur_[i] = 0;
    }
    end_ = true;
    pos_++;
  }

private:
  std::vector<indices> is_;
  std::vector<size_t> cur_;
  size_t pos_;
  bool end_;
};

} // namespace detail

/**
 * This implementation is used when a dense numpy.ndarray is used to represent
 * the data.
 */
class row_major_dense_dataview : public dataview {
public:
  row_major_dense_dataview(const uint8_t *data,
                           const bool *mask,
                           const std::vector<size_t> &shape,
                           const runtime_type &type)
    : dataview(shape, type), data_(data), dataend_(),
      mask_(mask), stepsize_(type.size())
  {
    MICROSCOPES_DCHECK(data, "data cannot be null");
    multipliers_.push_back(1);
    auto rit = shape.rbegin();
    for (size_t i = 0; i < shape.size() - 1; ++i, ++rit)
      multipliers_.push_back(multipliers_.back() * (*rit));
    std::reverse(multipliers_.begin(), multipliers_.end());
    size_t nelems = 1;
    for (auto s : shape)
      nelems *= s;
    dataend_ = data + nelems * stepsize_;
  }

  value_accessor
  get(const std::vector<size_t> &indices) const override
  {
    MICROSCOPES_DCHECK(dims() == indices.size(), "invalid # of indices");
    for (size_t i = 0; i < dims(); i++)
      MICROSCOPES_DCHECK(indices[i] < shape_[i], "index out of bounds");
    return accessor(indices);
  }

  class slice_iterator_impl : public dataview::slice_iterator_impl {
    friend class row_major_dense_dataview;
  protected:
    slice_iterator_impl(const row_major_dense_dataview *px,
                        const detail::product &iter)
      : px_(px), iter_(iter)
    {
      for (;;) {
        if (iter_.end() || !px->accessor(iter_.value()).anymasked())
          break;
        iter_.next();
      }
    }

  public:
    std::unique_ptr<dataview::slice_iterator_impl>
    clone() const override
    {
      return std::unique_ptr<slice_iterator_impl>(
          new slice_iterator_impl(*this));
    }

    // XXX: false positives possible if not iterators over
    // the same view or the same slice (within the same view)
    bool
    equals(const dataview::slice_iterator_impl &that) const override
    {
      const auto &o = static_cast<const slice_iterator_impl &>(that);
      return iter_.pos() == o.iter_.pos();
    }

    void
    next() override
    {
      for (;;) {
        iter_.next();
        if (iter_.end() || !px_->accessor(iter_.value()).anymasked())
          break;
      }
    }

    const value_with_position_t &
    value() const override
    {
      // XXX: const_cast so we can mutate the storage
      const_cast<slice_iterator_impl *>(this)->storage_.first =
        iter_.value();
      const_cast<slice_iterator_impl *>(this)->storage_.second =
        px_->accessor(iter_.value());
      return storage_;
    }

  private:
    const row_major_dense_dataview *px_;
    detail::product iter_;
    value_with_position_t storage_;
  };

  slice_iterable
  slice(size_t dim, size_t idx) const override
  {
    MICROSCOPES_DCHECK(dim < dims(), "invalid dimension");
    MICROSCOPES_DCHECK(idx < shape_[dim], "invalid index");
    std::vector<detail::product::indices> is(dims());
    for (size_t i = 0; i < dims(); i++) {
      if (i == dim)
        is[i] = detail::product::indices(detail::product::SINGLETON, idx);
      else
        is[i] = detail::product::indices(detail::product::RANGE, shape_[i]);
    }

    detail::product begin_iter(is), end_iter(is);
    end_iter.setEnd();

    std::unique_ptr<dataview::slice_iterator_impl> begin(
        new slice_iterator_impl(
          this, begin_iter));

    std::unique_ptr<dataview::slice_iterator_impl> end(
        new slice_iterator_impl(
          this, end_iter));

    return slice_iterable(std::move(begin), std::move(end));
  }

private:

  inline value_accessor
  accessor(const std::vector<size_t> &indices) const
  {
    const size_t off = offset(indices);
    const uint8_t *px = data_ + stepsize_ * off;
    MICROSCOPES_ASSERT(px < dataend_);
    return value_accessor(
        px, mask_ ? (mask_ + off) : nullptr, type());
  }

  inline size_t
  offset(const std::vector<size_t> &indices) const
  {
    MICROSCOPES_ASSERT(indices.size() == dims());
    size_t off = 0;
    for (size_t i = 0; i < dims(); i++)
      off += indices[i] * multipliers_[i];
    return off;
  }

  const uint8_t *data_;
  const uint8_t *dataend_;
  const bool *mask_;
  size_t stepsize_;
  std::vector<size_t> multipliers_;
};

/**
 * both scipy.sparse.csc_matrix and scipy.sparse.csr_matrix are represented
 * with this implementation
 *
 * the storage cost is 2X the storage cost for an individual cs{c,r}_matrix,
 * but a slice operation is exactly linear in the number of non-zero entries
 * along the slice (row/col in the 2D case)
 *
 * we trade off this space for time; otherwise, a slice along the non-dominant
 * dimension would be linear in the number of **total** non-zero entries
 *
 * XXX(stephentu): Implementation limitation:
 * Note that currently, the zero (sparse) elements are treated as **missing**
 * data, rather than 0-valued data. that is, the data is equivalent to the
 * dense represetation **plus** masking all the zero (sparse) entries.
 */
class compressed_2darray : public dataview {
public:

  // we assume the inputs are consistent with each other
  // without making any effort to validate
  compressed_2darray(const uint8_t *csr_data,
                     const uint32_t *csr_indices,
                     const uint32_t *csr_indptr,
                     const uint8_t *csc_data,
                     const uint32_t *csc_indices,
                     const uint32_t *csc_indptr,
                     size_t rows,
                     size_t cols,
                     const runtime_type &type)
    : dataview({rows, cols}, type),
      csr_data_(csr_data),
      csr_indices_(csr_indices),
      csr_indptr_(csr_indptr),
      csc_data_(csc_data),
      csc_indices_(csc_indices),
      csc_indptr_(csc_indptr)
  {
  }

  template <bool IsRowFixed>
  class slice_iterator_impl : public dataview::slice_iterator_impl {
    friend class compressed_2darray;
  protected:
    slice_iterator_impl(unsigned fixed_idx,
                        const uint32_t *indices,
                        const uint8_t *data,
                        const runtime_type *type)
      : fixed_idx_(fixed_idx),
        indices_(indices),
        data_(data),
        type_(type)
    {
    }

  public:
    std::unique_ptr<dataview::slice_iterator_impl>
    clone() const override
    {
      return std::unique_ptr<slice_iterator_impl<IsRowFixed>>(
          new slice_iterator_impl<IsRowFixed>(*this));
    }

    // false positives possible if not same view or
    // not same slice
    bool
    equals(const dataview::slice_iterator_impl &that) const override
    {
      const auto &o =
        static_cast<const slice_iterator_impl<IsRowFixed> &>(that);
      return indices_ == o.indices_;
    }

    void
    next() override
    {
      indices_++;
      data_ += type_->size();
    }

    const value_with_position_t &
    value() const override
    {
      const_cast<slice_iterator_impl *>(this)->load();
      return storage_;
    }

  private:

    inline void
    load()
    {
      storage_.first.resize(2);
      if (IsRowFixed) {
        storage_.first[0] = fixed_idx_;
        storage_.first[1] = *indices_;
      } else {
        storage_.first[0] = *indices_;
        storage_.first[1] = fixed_idx_;
      }
      storage_.second = value_accessor(data_, nullptr, *type_);
    }

    unsigned fixed_idx_;
    const uint32_t *indices_;
    const uint8_t *data_;
    const runtime_type *type_;
    value_with_position_t storage_;
  };

  value_accessor
  get(const std::vector<size_t> &indices) const override
  {
    MICROSCOPES_DCHECK(indices.size() == 2, "bad size given");
    // XXX(stephentu): don't be lazy
    MICROSCOPES_UNIMPLEMENTED();
  }

  template <bool IsRowFixed>
  inline std::unique_ptr<dataview::slice_iterator_impl>
  construct(size_t idx,
            const uint32_t *indices,
            const uint8_t *data) const
  {
    std::unique_ptr<dataview::slice_iterator_impl> ptr(
        new slice_iterator_impl<IsRowFixed>(
          idx, indices, data, &type()));
    return std::move(ptr);
  }

  slice_iterable
  slice(size_t dim, size_t idx) const override
  {
    //std::cout << "slice(" << dim << ", " << idx << ")" << std::endl;
    MICROSCOPES_DCHECK(dim < dims(), "invalid dimension");
    MICROSCOPES_DCHECK(idx < shape_[dim], "invalid index");

    const bool row_fixed = (dim == 0);
    const uint32_t *indptr = row_fixed ? csr_indptr_ : csc_indptr_;
    const uint32_t *indices = row_fixed ? csr_indices_ : csc_indices_;
    const uint8_t *data = row_fixed ? csr_data_ : csc_data_;
    const size_t sz = type().size();

    const uint32_t *begin_indices = &indices[indptr[idx]];
    const uint32_t *end_indices = &indices[indptr[idx+1]];
    const uint8_t *begin_data = &data[sz*indptr[idx]];
    const uint8_t *end_data = &data[sz*indptr[idx+1]];

    if (row_fixed) {
      auto begin = construct<true>(idx, begin_indices, begin_data);
      auto end = construct<true>(idx, end_indices, end_data);
      return slice_iterable(std::move(begin), std::move(end));
    } else {
      auto begin = construct<false>(idx, begin_indices, begin_data);
      auto end = construct<false>(idx, end_indices, end_data);
      return slice_iterable(std::move(begin), std::move(end));
    }
  }

  inline size_t rows() const { return shape()[0]; }
  inline size_t cols() const { return shape()[1]; }

private:
  const uint8_t *csr_data_;
  const uint32_t *csr_indices_;
  const uint32_t *csr_indptr_;
  const uint8_t *csc_data_;
  const uint32_t *csc_indices_;
  const uint32_t *csc_indptr_;
};

// XXX(stephentu): we need a good implementation for sparse n-ary relations
// (when n > 2). we'll most likely use something like an (indices, data)
// representation

} // namespace relation
} // namespace common
} // namespace microscopes
