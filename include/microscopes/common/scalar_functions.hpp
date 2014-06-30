#pragma once

#include <microscopes/common/typedefs.hpp>
#include <cmath>

namespace microscopes {
namespace common {

static inline scalar_1d_float_fn
log_exponential(float lambda)
{
  const float log_lambda = logf(lambda);
  return [lambda, log_lambda](float x) {
    return log_lambda - lambda * x;
  };
}

} // namespace common
} // namespace microscopes
