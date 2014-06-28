#pragma once

// sigh, the enum must be neither
//
//   (A) a C++11 class enum nor
//   (B) nested within a class
//
// since cython can handle neither of them particularly well (if even)

#define RUNTIME_TYPE_INFO(x) \
  x(TYPE_INFO_B) \
  x(TYPE_INFO_I8) \
  x(TYPE_INFO_I16) \
  x(TYPE_INFO_I32) \
  x(TYPE_INFO_I64) \
  x(TYPE_INFO_F32) \
  x(TYPE_INFO_F64)

// WARNING: don't name this type_info otherwise might class with std::type_info
enum runtime_type_info {
#define _ENUM_CASE(name) name,
  RUNTIME_TYPE_INFO(_ENUM_CASE)
#undef _ENUM_CASE
  TYPE_INFO_NELEMS,
};
