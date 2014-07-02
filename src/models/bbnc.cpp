#include <microscopes/models/bbnc.hpp>
#include <microscopes/io/schema.pb.h>
#include <distributions/random.hpp>

#include <stdexcept>
#include <sstream>
#include <cmath>

using namespace std;
using namespace distributions;
using namespace microscopes::io;
using namespace microscopes::common;
using namespace microscopes::models;

typedef BetaBernoulliNonConj::Group group_message_type;
typedef BetaBernoulliNonConj::Shared shared_message_type;

void
bbnc_feature_group::add_value(const model &m, const row_accessor &value, rng_t &rng)
{
  if (value.get<bool>())
    heads_++;
  else
    tails_++;
}

void
bbnc_feature_group::remove_value(const model &m, const row_accessor &value, rng_t &rng)
{
  if (value.get<bool>()) {
    MICROSCOPES_ASSERT(heads_ > 0);
    heads_--;
  } else {
    MICROSCOPES_ASSERT(tails_ > 0);
    tails_--;
  }
}

float
bbnc_feature_group::score_value(const model &m, const row_accessor &value, rng_t &rng) const
{
  return value.get<bool>() ? logf(p_) : logf(1.-p_);
}

static inline float
lbetaf(float a, float b)
{
  return lgammaf(a) + lgammaf(b) - lgammaf(a + b);
}

float
bbnc_feature_group::score_data(const model &m, rng_t &rng) const
{
  const float alpha = static_cast<const bbnc_model &>(m).alpha_;
  const float beta = static_cast<const bbnc_model &>(m).beta_;
  const float score_prior =
    (alpha-1.)*logf(p_) + (beta-1.)*logf(1.-p_) - lbetaf(alpha, beta);
  const float score_likelihood =
    float(heads_)*logf(p_) + float(tails_)*logf(1.-p_);
  return score_prior + score_likelihood;
}

void
bbnc_feature_group::sample_value(const model &m, row_mutator &value, rng_t &rng) const
{
  value.set<bool>(sample_bernoulli(rng, p_));
}

suffstats_bag_t
bbnc_feature_group::get_ss() const
{
  group_message_type m;
  m.set_p(p_);
  ostringstream out;
  m.SerializeToOstream(&out);
  return out.str();
}

void
bbnc_feature_group::set_ss(const suffstats_bag_t &ss)
{
  istringstream inp(ss);
  group_message_type m;
  m.ParseFromIstream(&inp);
  p_ = m.p();
}

shared_ptr<feature_group>
bbnc_model::create_feature_group(rng_t &rng) const
{
  return make_shared<bbnc_feature_group>(sample_beta(rng, alpha_, beta_));
}

hyperparam_bag_t
bbnc_model::get_hp() const
{
  shared_message_type m;
  m.set_alpha(alpha_);
  m.set_beta(beta_);
  ostringstream out;
  m.SerializeToOstream(&out);
  return out.str();
}

void
bbnc_model::set_hp(const hyperparam_bag_t &hp)
{
  istringstream inp(hp);
  shared_message_type m;
  m.ParseFromIstream(&inp);
  alpha_ = m.alpha();
  beta_ = m.beta();
}

void
bbnc_model::set_hp(const model &m)
{
  *this = static_cast<const bbnc_model &>(m);
}

void *
bbnc_model::get_hp_raw_ptr(const string &key)
{
  if (key == "alpha")
    return &alpha_;
  if (key == "beta")
    return &beta_;
  throw runtime_error("unknown key: " + key);
}

runtime_type_info
bbnc_model::get_runtime_type_info() const
{
  return TYPE_INFO_B;
}

string
bbnc_model::debug_str() const
{
  ostringstream oss;
  oss << "{alpha:" << alpha_ << ",beta:" << beta_ << "}";
  return oss.str();
}
