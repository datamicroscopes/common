#include <microscopes/models/dm.hpp>
#include <distributions/special.hpp>

using namespace std;
using namespace distributions;
using namespace microscopes::common;
using namespace microscopes::models;

void
dm_group::add_value(const hypers &m, const value_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.shape() == categories());
  unsigned count_sum = 0;
  for (size_t i = 0; i < categories(); i++) {
    const unsigned ni = value.get<unsigned>(i);
    count_sum += ni;
    counts_[i] += ni;
    ratio_ -= fast_lgamma(ni + 1);
  }
  ratio_ += fast_lgamma(count_sum + 1);
}

void
dm_group::remove_value(const hypers &m, const value_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.shape() == categories());
  unsigned count_sum = 0;
  for (size_t i = 0; i < categories(); i++) {
    const unsigned ni = value.get<unsigned>(i);
    count_sum += ni;
    MICROSCOPES_ASSERT(counts_[i] >= ni);
    counts_[i] -= ni;
    ratio_ += fast_lgamma(ni + 1);
  }
  ratio_ -= fast_lgamma(count_sum + 1);
}

float
dm_group::score_value(const hypers &m, const value_accessor &value, rng_t &rng) const
{
  const dm_hypers &h = static_cast<const dm_hypers &>(m);
  MICROSCOPES_ASSERT(categories() == h.categories());
  MICROSCOPES_ASSERT(value.shape() == categories());
  float score = 0.;
  unsigned x_sum = 0;
  float a_sum = 0.;
  unsigned n_sum = 0;
  for (size_t i = 0; i < categories(); i++) {
    const unsigned xi = value.get<unsigned>(i);
    const float ai = h.alphas()[i];
    const unsigned ni = counts_[i];
    x_sum += xi;
    a_sum += ai;
    n_sum += ni;
    score -= fast_lgamma(xi + 1);
    score += xi * fast_log(ai + ni);
  }
  score += fast_lgamma(x_sum + 1);
  score -= x_sum * fast_log(a_sum + n_sum);
  return score;
}

float
dm_group::score_data(const hypers &m, rng_t &rng) const
{
  const dm_hypers &h = static_cast<const dm_hypers &>(m);
  MICROSCOPES_ASSERT(categories() == h.categories());
  float score = ratio_;
  float alpha_sum = 0.;
  unsigned count_sum = 0;
  for (size_t i = 0; i < categories(); i++) {
    const float alpha = h.alphas()[i];
    const unsigned count = counts_[i];
    alpha_sum += alpha;
    count_sum += count;
    score += fast_lgamma(count + alpha)
           - fast_lgamma(alpha);
  }
  score += fast_lgamma(alpha_sum)
         - fast_lgamma(alpha_sum + count_sum);
  return score;
}

void
dm_group::sample_value(const hypers &m, value_mutator &value, rng_t &rng) const
{
  const dm_hypers &h = static_cast<const dm_hypers &>(m);
  (void)h;
  MICROSCOPES_ASSERT(categories() == h.categories());
  MICROSCOPES_ASSERT(value.shape() == categories());
  // XXX: we need a way to specify n, the # of samples from a categorical
  // distribution!
  //
  // sample_value() currently doesn't fit into our framework
  throw runtime_error("multinomial sampling unimplemented");
}
