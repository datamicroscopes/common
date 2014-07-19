#pragma once

#include <microscopes/common/assert.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/models/base.hpp>

namespace microscopes {
namespace models {

class bbnc_group : public group {
  friend class bbnc_hypers;
public:
  bbnc_group(float p)
    : heads_(), tails_(), p_(p)
  {
    MICROSCOPES_ASSERT(p >= 0.0 && p <= 1.0);
  }

  void add_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override;
  void remove_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override;
  float score_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) const override;
  float score_data(const hypers &m, common::rng_t &rng) const override;
  void sample_value(const hypers &m, common::value_mutator &value, common::rng_t &rng) const override;

  common::suffstats_bag_t get_ss() const override;
  void set_ss(const common::suffstats_bag_t &ss) override;
  void set_ss(const group &g) override;

  common::value_mutator get_ss_mutator(const std::string &key) override;
  std::string debug_str() const override;

protected:
  size_t heads_;
  size_t tails_;
  float p_;
};

class bbnc_hypers : public hypers {
  friend class bbnc_group;
public:
  bbnc_hypers() : alpha_(), beta_() {}

  std::shared_ptr<group> create_group(common::rng_t &rng) const override;

  common::hyperparam_bag_t get_hp() const override;
  void set_hp(const common::hyperparam_bag_t &hp) override;
  void set_hp(const hypers &m) override;
  common::value_mutator get_hp_mutator(const std::string &key) override;

  std::string debug_str() const override;

  static inline size_t
  CreateFeatureGroupInvocations()
  {
    return CreateFeatureGroupInvocations_;
  }

protected:
  float alpha_;
  float beta_;

  static size_t CreateFeatureGroupInvocations_;
};

class bbnc_model : public model {
public:
  std::shared_ptr<hypers>
  create_hypers() const
  {
    return std::make_shared<bbnc_hypers>();
  }
  common::runtime_type get_runtime_type() const override;
};

} // namespace hyperss
} // namespace microscopes
