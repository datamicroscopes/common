#pragma once

#include <stdexcept>

#define NEVER_INLINE  __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))
#define UNUSED __attribute__((unused))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define MICROSCOPES_DCHECK(expr, msg) \
  do { \
    if (unlikely(!(expr))) \
      throw ::std::runtime_error(msg); \
  } while (0)
