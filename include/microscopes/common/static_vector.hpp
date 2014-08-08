#pragma once

// C++ wrapper around a statically sized array
//
// Based on:
// https://github.com/stephentu/silo/blob/master/static_vector.h

#include <algorithm>
#include <type_traits>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/assert.hpp>

namespace microscopes {
namespace common {

template <typename T, size_t MaxSize>
class static_vector {

  static const bool is_trivially_destructible =
    std::is_trivially_destructible<T>::value;

  // not supported on g++-4.8
  //static const bool is_trivially_copyable =
  //  std::is_trivially_copyable<T>::value;

  static const bool is_trivially_copyable =
    std::is_scalar<T>::value;

public:

  typedef T value_type;
  typedef T & reference;
  typedef const T & const_reference;
  typedef size_t size_type;

  inline static_vector() : n(0) {}
  inline ~static_vector() { clear(); }

  inline static_vector(const static_vector &that)
    : n(0)
  {
    assignFrom(that);
  }

  // the next two constructors are not efficient

  template <typename ForwardIterator>
  inline static_vector(ForwardIterator begin, ForwardIterator end)
    : n(0)
  {
    while (begin != end)
      push_back(*begin++);
  }

  static_vector(std::initializer_list<T> l)
    : n(0)
  {
    for (auto &p : l)
      push_back(p);
  }

  static_vector &
  operator=(const static_vector &that)
  {
    assignFrom(that);
    return *this;
  }

  inline size_t
  size() const
  {
    return n;
  }

  inline bool
  empty() const
  {
    return !n;
  }

  inline reference
  front()
  {
    MICROSCOPES_ASSERT(n > 0);
    MICROSCOPES_ASSERT(n <= MaxSize);
    return *ptr();
  }

  inline const_reference
  front() const
  {
    return const_cast<static_vector *>(this)->front();
  }

  inline reference
  back()
  {
    MICROSCOPES_ASSERT(n > 0);
    MICROSCOPES_ASSERT(n <= MaxSize);
    return ptr()[n - 1];
  }

  inline const_reference
  back() const
  {
    return const_cast<static_vector *>(this)->back();
  }

  inline void
  pop_back()
  {
    MICROSCOPES_ASSERT(n > 0);
    if (!is_trivially_destructible)
      ptr()[n - 1].~T();
    n--;
  }

  inline void
  push_back(const T &obj)
  {
    emplace_back(obj);
  }

  inline void
  push_back(T &&obj)
  {
    emplace_back(std::move(obj));
  }

  // C++11 goodness- a strange syntax this is

  template <class... Args>
  inline void
  emplace_back(Args &&... args)
  {
    MICROSCOPES_ASSERT(n < MaxSize);
    new (&(ptr()[n++])) T(std::forward<Args>(args)...);
  }

  inline reference
  operator[](int i)
  {
    return ptr()[i];
  }

  inline const_reference
  operator[](int i) const
  {
    return const_cast<static_vector *>(this)->operator[](i);
  }

  void
  clear()
  {
    if (!is_trivially_destructible)
      for (size_t i = 0; i < n; i++)
        ptr()[i].~T();
    n = 0;
  }

  inline void
  reserve(size_t n)
  {
  }

  inline void
  resize(size_t n, value_type val = value_type())
  {
    MICROSCOPES_ASSERT(n <= MaxSize);
    if (n > this->n) {
      // expand
      while (this->n < n)
        new (&ptr()[this->n++]) T(val);
    } else if (n < this->n) {
      // shrink
      while (this->n > n) {
        if (!is_trivially_destructible)
          ptr()[this->n - 1].~T();
        this->n--;
      }
    }
  }

  // non-standard API

  template <typename Compare = std::less<T>>
  inline void
  sort(Compare c = Compare())
  {
    std::sort(begin(), end(), c);
  }

private:

  template <typename ObjType>
  class iterator_ : public std::iterator<std::bidirectional_iterator_tag, ObjType> {
    friend class static_vector;
  public:
    inline iterator_() : p(0) {}

    template <typename O>
    inline iterator_(const iterator_<O> &other)
      : p(other.p)
    {}

    inline ObjType &
    operator*() const
    {
      return *p;
    }

    inline ObjType *
    operator->() const
    {
      return p;
    }

    inline bool
    operator==(const iterator_ &o) const
    {
      return p == o.p;
    }

    inline bool
    operator!=(const iterator_ &o) const
    {
      return !operator==(o);
    }

    inline bool
    operator<(const iterator_ &o) const
    {
      return p < o.p;
    }

    inline bool
    operator>=(const iterator_ &o) const
    {
      return !operator<(o);
    }

    inline bool
    operator>(const iterator_ &o) const
    {
      return p > o.p;
    }

    inline bool
    operator<=(const iterator_ &o) const
    {
      return !operator>(o);
    }

    inline iterator_ &
    operator+=(int n)
    {
      p += n;
      return *this;
    }

    inline iterator_ &
    operator-=(int n)
    {
      p -= n;
      return *this;
    }

    inline iterator_
    operator+(int n) const
    {
      iterator_ cpy = *this;
      return cpy += n;
    }

    inline iterator_
    operator-(int n) const
    {
      iterator_ cpy = *this;
      return cpy -= n;
    }

    inline intptr_t
    operator-(const iterator_ &o) const
    {
      return p - o.p;
    }

    inline iterator_ &
    operator++()
    {
      ++p;
      return *this;
    }

    inline iterator_
    operator++(int)
    {
      iterator_ cur = *this;
      ++(*this);
      return cur;
    }

    inline iterator_ &
    operator--()
    {
      --p;
      return *this;
    }

    inline iterator_
    operator--(int)
    {
      iterator_ cur = *this;
      --(*this);
      return cur;
    }

  protected:
    inline iterator_(ObjType *p) : p(p) {}

  private:
    ObjType *p;
  };

public:

  typedef iterator_<T> iterator;
  typedef iterator_<const T> const_iterator;

  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  inline iterator
  begin()
  {
    return iterator(ptr());
  }

  inline const_iterator
  begin() const
  {
    return const_iterator(ptr());
  }

  inline iterator
  end()
  {
    return iterator(endptr());
  }

  inline const_iterator
  end() const
  {
    return const_iterator(endptr());
  }

  inline reverse_iterator
  rbegin()
  {
    return reverse_iterator(end());
  }

  inline const_reverse_iterator
  rbegin() const
  {
    return const_reverse_iterator(end());
  }

  inline reverse_iterator
  rend()
  {
    return reverse_iterator(begin());
  }

  inline const_reverse_iterator
  rend() const
  {
    return const_reverse_iterator(begin());
  }

private:
  void
  assignFrom(const static_vector &that)
  {
    if (unlikely(this == &that))
      return;
    clear();
    MICROSCOPES_ASSERT(that.n <= MaxSize);
    if (is_trivially_copyable) {
      MICROSCOPES_MEMCPY(ptr(), that.ptr(), that.n * sizeof(T));
    } else {
      for (size_t i = 0; i < that.n; i++)
        new (&(ptr()[i])) T(that.ptr()[i]);
    }
    n = that.n;
  }

  inline ALWAYS_INLINE T *
  ptr()
  {
    return reinterpret_cast<T *>(&elems_buf[0]);
  }

  inline ALWAYS_INLINE const T *
  ptr() const
  {
    return reinterpret_cast<const T *>(&elems_buf[0]);
  }

  inline ALWAYS_INLINE T *
  endptr()
  {
    return ptr() + n;
  }

  inline ALWAYS_INLINE const T *
  endptr() const
  {
    return ptr() + n;
  }

  size_t n;
  char elems_buf[sizeof(T) * MaxSize];
};

} // namespace common
} // namespace microscopes

// operators

template <typename T, size_t M>
inline bool
operator==(const microscopes::common::static_vector<T, M> &x,
           const microscopes::common::static_vector<T, M> &y)
{
  return (x.size() == y.size()) && std::equal(x.begin(), x.end(), y.begin());
}

template <typename T, size_t M>
inline bool
operator!=(const microscopes::common::static_vector<T, M> &x,
           const microscopes::common::static_vector<T, M> &y)
{
  return !(x == y);
}

template <typename T, size_t M>
inline bool
operator<(const microscopes::common::static_vector<T, M> &x,
          const microscopes::common::static_vector<T, M> &y)
{
  return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}

template <typename T, size_t M>
inline bool
operator>(const microscopes::common::static_vector<T, M> &x,
          const microscopes::common::static_vector<T, M> &y)
{
  return y < x;
}

template <typename T, size_t M>
inline bool
operator<=(const microscopes::common::static_vector<T, M> &x,
           const microscopes::common::static_vector<T, M> &y)
{
  return !(y < x);
}

template <typename T, size_t M>
inline bool
operator>=(const microscopes::common::static_vector<T, M> &x,
           const microscopes::common::static_vector<T, M> &y)
{
  return !(x < y);
}

// XXX: std::hash specialization
