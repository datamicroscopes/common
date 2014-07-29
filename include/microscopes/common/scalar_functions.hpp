#pragma once

#include <microscopes/common/macros.hpp>
#include <distributions/special.hpp> // for M_PIf and fast_log()
#include <limits>
#include <cmath>
#include <vector>

namespace microscopes {
namespace common {

class scalar_fn {
public:
  scalar_fn() : fn_(), input_dim_() {}
  scalar_fn(std::function<float(const std::vector<float> &)> fn,
            size_t input_dim)
    : fn_(fn), input_dim_(input_dim)
  {}

  inline float
  operator()(const std::vector<float> &args)
  {
    MICROSCOPES_DCHECK(args.size() == input_dim_, "wrong # of args");
    return fn_(args);
  }

  inline size_t input_dim() const { return input_dim_; }

private:
  std::function<float(const std::vector<float> &)> fn_;
  size_t input_dim_;
};

static inline scalar_fn
log_exponential(float lambda)
{
  const float log_lambda = logf(lambda);
  return scalar_fn(
      [lambda, log_lambda](const std::vector<float> &args) {
        const float x = args.front();
        if (x < 0)
          return -std::numeric_limits<float>::infinity();
        return log_lambda - lambda * x;
      }, 1);
}

static inline scalar_fn
log_normal(float mu, float sigma2)
{
  const float lgC = -0.5 * logf(2.*M_PIf*sigma2);
  const float one_half_inv_sigma2 = 0.5 * 1./sigma2;
  return scalar_fn(
      [mu, sigma2, lgC, one_half_inv_sigma2](const std::vector<float> &args) {
        const float x = args.front();
        const float diff = x - mu;
        return lgC - one_half_inv_sigma2 * diff * diff;
      }, 1);
}

static inline scalar_fn
log_noninformative_beta_prior()
{
  // a non-informative (proper) prior for the beta distribution
  // http://iacs-courses.seas.harvard.edu/courses/am207/blog/lecture-9.html
  return scalar_fn(
      [](const std::vector<float> &args) {
        const float alpha = args[0];
        const float beta = args[1];
        if (alpha <= 0.0 || beta <= 0.0)
          return -std::numeric_limits<float>::infinity();
        return -2.5f*distributions::fast_log(alpha + beta);
      }, 2);
}

} // namespace common
} // namespace microscopes
