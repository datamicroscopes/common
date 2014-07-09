#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/macros.hpp>

#include <algorithm>
#include <iterator>
#include <memory>

namespace microscopes {
namespace common {
namespace sparse_ndarray {

class dataview {
public:

  typedef std::pair<std::vector<size_t>, value_accessor> value_with_position_t;

  class slice_iterator_impl {
  public:
    virtual ~slice_iterator_impl() {}
    virtual std::unique_ptr<slice_iterator_impl> clone() const = 0;
    virtual bool equals(const slice_iterator_impl &) const = 0;
    virtual bool end() const = 0;
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
      impl_ = std::move(that.impl_->clone());
      return *this;
    }

    inline bool
    operator==(const slice_iterator &that) const
    {
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
      impl_->next(); return *this;
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
    for (auto s : shape)
      MICROSCOPES_DCHECK(s, "empty-dimesion not allowed");
  }

  virtual ~dataview() {}

  inline size_t dims() const { return shape_.size(); }
  inline const std::vector<size_t> & shape() const { return shape_; }
  inline const runtime_type & type() const { return type_; }

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
 * useful for testing, but horrible in performance
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
      MICROSCOPES_DCHECK(indices[i] < shape_[i], "index out of bounds");
    return accessor(indices);
  }

  class slice_iterator_impl : public dataview::slice_iterator_impl {
    friend class row_major_dense_dataview;
  protected:
    slice_iterator_impl(
      const row_major_dense_dataview *px,
      size_t dim,
      size_t idx,
      const detail::product &iter)
      : px_(px), dim_(dim), idx_(idx), iter_(iter)
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
      return std::unique_ptr<slice_iterator_impl>(new slice_iterator_impl(*this));
    }

    bool
    equals(const dataview::slice_iterator_impl &that) const override
    {
      const auto &o = static_cast<const slice_iterator_impl &>(that);
      return px_ == o.px_ &&
             dim_ == o.dim_ &&
             idx_ == o.idx_ &&
             iter_.pos() == o.iter_.pos();
    }

    bool end() const override { return iter_.end(); }

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
      const_cast<slice_iterator_impl *>(this)->storage_.first = iter_.value();
      const_cast<slice_iterator_impl *>(this)->storage_.second = px_->accessor(iter_.value());
      return storage_;
    }

  private:
    const row_major_dense_dataview *px_;
    size_t dim_;
    size_t idx_;
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
          this,
          dim,
          idx,
          begin_iter));

    std::unique_ptr<dataview::slice_iterator_impl> end(
        new slice_iterator_impl(
          this,
          dim,
          idx,
          end_iter));

    return std::move(
        slice_iterable(std::move(begin), std::move(end)));
  }

private:

  inline value_accessor
  accessor(const std::vector<size_t> &indices) const
  {
    const size_t off = offset(indices);
    return value_accessor(
        data_ + stepsize_ * off,
        mask_ ? (mask_ + off) : nullptr,
        type());
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
  const bool *mask_;
  size_t stepsize_;
  std::vector<size_t> multipliers_;
};

} // namespace sparse_ndarray
} // namespace common
} // namespace microscopes
