#pragma once

#include <microscopes/common/typedefs.hpp>
#include <distributions/special.hpp> // for M_PIf
#include <limits>
#include <cmath>

namespace microscopes {
namespace common {

static inline scalar_1d_float_fn
log_exponential(float lambda)
{
  const float log_lambda = logf(lambda);
  return [lambda, log_lambda](float x) {
    if (x < 0)
      return std::numeric_limits<float>::infinity();
    return log_lambda - lambda * x;
  };
}

static inline scalar_1d_float_fn
log_normal(float mu, float sigma2)
{
  const float lgC = -0.5 * logf(2.*M_PIf*sigma2);
  const float one_half_inv_sigma2 = 0.5 * 1./sigma2;
  return [mu, sigma2, lgC, one_half_inv_sigma2](float x) {
    const float diff = x - mu;
    return lgC - one_half_inv_sigma2 * diff * diff;
  };
}

} // namespace common
} // namespace microscopes
