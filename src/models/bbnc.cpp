#include <microscopes/models/bbnc.hpp>
#include <microscopes/io/schema.pb.h>
#include <microscopes/common/util.hpp>
#include <distributions/random.hpp>
#include <distributions/special.hpp>

#include <stdexcept>
#include <sstream>
#include <cmath>
#include <limits>

using namespace std;
using namespace distributions;
using namespace microscopes::io;
using namespace microscopes::common;
using namespace microscopes::models;

typedef BetaBernoulliNonConj::Group group_message_type;
typedef BetaBernoulliNonConj::Shared shared_message_type;

void
bbnc_group::add_value(const hypers &m, const value_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.shape() == 1);
  MICROSCOPES_ASSERT(!value.ismasked(0));
  if (value.get<bool>(0))
    heads_++;
  else
    tails_++;
}

void
bbnc_group::remove_value(const hypers &m, const value_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.shape() == 1);
  MICROSCOPES_ASSERT(!value.ismasked(0));
  if (value.get<bool>(0)) {
    MICROSCOPES_ASSERT(heads_ > 0);
    heads_--;
  } else {
    MICROSCOPES_ASSERT(tails_ > 0);
    tails_--;
  }
}

float
bbnc_group::score_value(const hypers &m, const value_accessor &value, rng_t &rng) const
{
  MICROSCOPES_ASSERT(p_ >= 0.0 && p_ <= 1.0);
  MICROSCOPES_ASSERT(value.shape() == 1);
  MICROSCOPES_ASSERT(!value.ismasked(0));
  return value.get<bool>(0) ? fast_log(p_) : fast_log(1.-p_);
}

static inline float
fast_lbeta(float a, float b)
{
  return fast_lgamma(a) + fast_lgamma(b) - fast_lgamma(a + b);
}

float
bbnc_group::score_data(const hypers &m, rng_t &rng) const
{
  if (p_ < 0.0 || p_ > 1.0)
    return -numeric_limits<float>::infinity();
  const float alpha = static_cast<const bbnc_hypers &>(m).alpha_;
  const float beta = static_cast<const bbnc_hypers &>(m).beta_;
  const float score_prior =
    (alpha-1.)*fast_log(p_) + (beta-1.)*fast_log(1.-p_) - fast_lbeta(alpha, beta);
  const float score_likelihood =
    float(heads_)*fast_log(p_) + float(tails_)*fast_log(1.-p_);
  return score_prior + score_likelihood;
}

void
bbnc_group::sample_value(const hypers &m, value_mutator &value, rng_t &rng) const
{
  MICROSCOPES_ASSERT(p_ >= 0.0 && p_ <= 1.0);
  MICROSCOPES_ASSERT(value.shape() == 1);
  value.set<bool>(sample_bernoulli(rng, p_), 0);
}

suffstats_bag_t
bbnc_group::get_ss() const
{
  group_message_type m;
  m.set_p(p_);
  return util::protobuf_to_string(m);
}

void
bbnc_group::set_ss(const suffstats_bag_t &ss)
{
  group_message_type m;
  util::protobuf_from_string(m, ss);
  p_ = m.p();
}

void
bbnc_group::set_ss(const group &ss)
{
  *this = static_cast<const bbnc_group &>(ss);
}

value_mutator
bbnc_group::get_ss_mutator(const string &key)
{
  if (key == "p")
    return value_mutator(&p_);
  throw runtime_error("unknown key: " + key);
}

string
bbnc_group::debug_str() const
{
  ostringstream oss;
  oss << "{p:" << p_ << "}";
  return oss.str();
}

shared_ptr<group>
bbnc_hypers::create_group(rng_t &rng) const
{
  CreateFeatureGroupInvocations_++;
  return make_shared<bbnc_group>(sample_beta(rng, alpha_, beta_));
}

hyperparam_bag_t
bbnc_hypers::get_hp() const
{
  shared_message_type m;
  m.set_alpha(alpha_);
  m.set_beta(beta_);
  return util::protobuf_to_string(m);
}

void
bbnc_hypers::set_hp(const hyperparam_bag_t &hp)
{
  shared_message_type m;
  util::protobuf_from_string(m, hp);
  alpha_ = m.alpha();
  beta_ = m.beta();
}

void
bbnc_hypers::set_hp(const hypers &m)
{
  *this = static_cast<const bbnc_hypers &>(m);
}

value_mutator
bbnc_hypers::get_hp_mutator(const string &key)
{
  if (key == "alpha")
    return value_mutator(&alpha_);
  if (key == "beta")
    return value_mutator(&beta_);
  throw runtime_error("unknown key: " + key);
}

string
bbnc_hypers::debug_str() const
{
  ostringstream oss;
  oss << "{alpha:" << alpha_ << ",beta:" << beta_ << "}";
  return oss.str();
}

size_t
bbnc_hypers::CreateFeatureGroupInvocations_ = 0;

runtime_type
bbnc_model::get_runtime_type() const
{
  return runtime_type(TYPE_B);
}
