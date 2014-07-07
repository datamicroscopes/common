#pragma once

#include <distributions/special.hpp>

namespace microscopes {
namespace common {

struct special {

  /**
   * http://en.wikipedia.org/wiki/Multivariate_gamma_function
   */
  static float
  lmultigamma(unsigned d, float a)
  {
    MICROSCOPES_ASSERT(d > 0);
    const float term1 = 0.25*float(d*(d-1))*1.1447298858494002 /* log(pi) */;
    float term2 = 0.;
    for (unsigned j = 1; j <= d; j++)
      term2 += distributions::fast_lgamma(a + 0.5*float(1-j));
    return term1 + term2;
  }

};

} // namespace common
} // namespace microscopes
