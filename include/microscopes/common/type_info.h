#pragma once

// sigh, the enum must be neither
//
//   (A) a C++11 class enum nor
//   (B) nested within a class
//
// since cython can handle neither of them particularly well (if even)

// WARNING: don't name this type_info otherwise might class with std::type_info
enum runtime_type_info {
  TYPE_INFO_B,
  TYPE_INFO_I8,
  TYPE_INFO_I16,
  TYPE_INFO_I32,
  TYPE_INFO_I64,
  TYPE_INFO_F32,
  TYPE_INFO_F64,
  TYPE_INFO_NELEMS,
};
