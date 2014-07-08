#include <microscopes/common/runtime_type.hpp>

using namespace microscopes::common;

const size_t runtime_type_traits::PrimitiveTypeSizes_[] = {
#define SIZE_EXPR(tname, rname) sizeof(tname),
PRIMITIVE_TYPE_MAPPINGS(SIZE_EXPR)
#undef SIZE_EXPR
};
