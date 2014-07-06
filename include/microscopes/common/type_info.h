#pragma once

// sigh, the enum must be neither
//
//   (A) a C++11 class enum nor
//   (B) nested within a class
//
// since cython can handle neither of them particularly well (if even)

#define PRIMITIVE_TYPES(x) \
  x(TYPE_B) \
  x(TYPE_I8) \
  x(TYPE_U8) \
  x(TYPE_I16) \
  x(TYPE_U16) \
  x(TYPE_I32) \
  x(TYPE_U32) \
  x(TYPE_I64) \
  x(TYPE_U64) \
  x(TYPE_F32) \
  x(TYPE_F64)

#define PRIMITIVE_TYPE_MAPPINGS(x) \
  x(bool, TYPE_B) \
  x(int8_t, TYPE_I8) \
  x(uint8_t, TYPE_U8) \
  x(int16_t, TYPE_I16) \
  x(uint16_t, TYPE_U16) \
  x(int32_t, TYPE_I32) \
  x(uint32_t, TYPE_U32) \
  x(int64_t, TYPE_I64) \
  x(uint64_t, TYPE_U64) \
  x(float, TYPE_F32) \
  x(double, TYPE_F64)

static_assert(sizeof(float) == 4, "platform assumption violated");
static_assert(sizeof(double) == 8, "platform assumption violated");

enum primitive_type {
#define _ENUM_CASE(name) name,
  PRIMITIVE_TYPES(_ENUM_CASE)
#undef _ENUM_CASE
  TYPE_NELEMS,
};
