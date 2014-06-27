#include <microscopes/common/type_helper.hpp>

using namespace microscopes::common;

// order must match type_info.h
// XXX: do some macro magic to make this easier
const size_t runtime_type_traits::TypeSizes_[] = {
  _info<bool>::size,
  _info<int8_t>::size,
  _info<int16_t>::size,
  _info<int32_t>::size,
  _info<int64_t>::size,
  _info<float>::size,
  _info<double>::size,
};
