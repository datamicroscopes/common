#include <microscopes/models/distributions.hpp>
using namespace distributions;

namespace distributions {
template struct DirichletDiscrete<128>;
}

namespace microscopes {
namespace models {

#define DISTRIB_EXPLICIT_INSTANTIATE(x) \
  template class distributions_model< x >;
DISTRIB_FOR_EACH_DISTRIBUTION(DISTRIB_EXPLICIT_INSTANTIATE)
#undef DISTRIB_EXPLICIT_INSTANTIATE

} // namespace models
} // namespace microscopes
