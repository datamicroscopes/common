#pragma once

#include <stdexcept>
#include <string>
#include <cassert>

#define NEVER_INLINE  __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))
#define UNUSED __attribute__((unused))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define compiler_barrier() asm volatile("" ::: "memory")

#ifdef DEBUG_MODE
  #define MICROSCOPES_DCHECK(expr, msg) \
    do { \
      if (unlikely(!(expr))) \
        throw ::std::runtime_error(msg); \
    } while (0)
#else
  #define MICROSCOPES_DCHECK(expr, msg) ((void)0)
#endif

// from https://code.google.com/p/protobuf/source/browse/trunk/src/google/protobuf/stubs/common.h
#define MICROSCOPES_ARRAYSIZE(a) \
  ((sizeof(a) / sizeof(*(a))) / \
      static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

#define _STRINGIFY(x) #x

/**
 * __LINE__ comes from the preprocessor as an int.
 * Stringify hack comes from:
 * http://www.decompile.com/cpp/faq/file_and_line_error_string.htm
 */
#define _LINEHACK(x) _STRINGIFY(x)

#define _SOURCE_INFO \
  (::std::string(__PRETTY_FUNCTION__) + \
   ::std::string(" (" __FILE__ ":" _LINEHACK(__LINE__) ")"))

#define MICROSCOPES_NOT_REACHABLE() \
  do { \
    throw ::std::runtime_error( \
      ::std::string("Should not be reached: ") + \
      ::std::string(_SOURCE_INFO)); \
  } while (0)

#ifdef NDEBUG
  #define ALWAYS_ASSERT(expr) (likely((expr)) ? (void)0 : abort())
#else
  #define ALWAYS_ASSERT(expr) assert((expr))
#endif

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)
  #define GCC_AT_LEAST_47 1
#else
  #define GCC_AT_LEAST_47 0
#endif

// g++-4.6 does not support override, so we define it to be a no-op
#if !defined(__clang__) && !GCC_AT_LEAST_47
  #define override
#endif
