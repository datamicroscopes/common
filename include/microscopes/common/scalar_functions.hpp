#pragma once

#include <microscopes/common/typedefs.hpp>
#include <microscopes/common/macros.hpp>
#include <distributions/special.hpp> // for M_PIf and fast_log()
#include <limits>
#include <cmath>

namespace microscopes {
namespace common {

static inline scalar_fn
log_exponential(float lambda)
{
  const float log_lambda = logf(lambda);
  return [lambda, log_lambda](const std::vector<float> &args) {
    MICROSCOPES_DCHECK( args.size() == 1, "expecting one arg");
    const float x = args.front();
    if (x < 0)
      return -std::numeric_limits<float>::infinity();
    return log_lambda - lambda * x;
  };
}

static inline scalar_fn
log_normal(float mu, float sigma2)
{
  const float lgC = -0.5 * logf(2.*M_PIf*sigma2);
  const float one_half_inv_sigma2 = 0.5 * 1./sigma2;
  return [mu, sigma2, lgC, one_half_inv_sigma2](const std::vector<float> &args) {
    MICROSCOPES_DCHECK( args.size() == 1, "expecting one arg");
    const float x = args.front();
    const float diff = x - mu;
    return lgC - one_half_inv_sigma2 * diff * diff;
  };
}

static inline scalar_fn
log_noninformative_beta_prior()
{
  return [](const std::vector<float> &args) {
    MICROSCOPES_DCHECK(args.size() == 2, "expecting two args");
    const float alpha = args[0];
    const float beta = args[1];
    return -2.5*distributions::fast_log(alpha + beta);
  };
}

} // namespace common
} // namespace microscopes
