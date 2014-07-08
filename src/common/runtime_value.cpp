#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/util.hpp>

using namespace std;
using namespace microscopes::common;

string
value_accessor::debug_str() const
{
  if (!type_.vec()) {
    return ismasked(0) ? "--" : runtime_type_traits::ToString(type_.t(), data_);
  } else {
    const size_t s = runtime_type_traits::PrimitiveTypeSize(type_.t());
    vector<string> strs;
    strs.reserve(type_.n());
    for (size_t j = 0; j < type_.n(); j++)
      strs.push_back(
          ismasked(j) ?
            "--" :
            runtime_type_traits::ToString(type_.t(), data_ + j * s));
    return util::to_string(strs);
  }
}
