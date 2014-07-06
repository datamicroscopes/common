#pragma once

#include <microscopes/common/assert.hpp>
#include <microscopes/models/base.hpp>

namespace microscopes {
namespace models {

class bbnc_feature_group : public feature_group {
  friend class bbnc_model;
public:
  bbnc_feature_group(float p)
    : heads_(), tails_(), p_(p)
  {
    MICROSCOPES_ASSERT(p >= 0.0 && p <= 1.0);
  }

  void add_value(const model &m, const common::row_accessor &value, common::rng_t &rng) override;
  void remove_value(const model &m, const common::row_accessor &value, common::rng_t &rng) override;
  float score_value(const model &m, const common::row_accessor &value, common::rng_t &rng) const override;
  float score_data(const model &m, common::rng_t &rng) const override;
  void sample_value(const model &m, common::row_mutator &value, common::rng_t &rng) const override;

  common::suffstats_bag_t get_ss() const override;
  void set_ss(const common::suffstats_bag_t &ss) override;

  void * get_ss_raw_ptr(const std::string &key) override;

protected:
  size_t heads_;
  size_t tails_;
  float p_;
};

class bbnc_model : public model {
  friend class bbnc_feature_group;
public:
  bbnc_model() : alpha_(), beta_() {}

  std::shared_ptr<feature_group> create_feature_group(common::rng_t &rng) const override;

  common::hyperparam_bag_t get_hp() const override;
  void set_hp(const common::hyperparam_bag_t &hp) override;
  void set_hp(const model &m) override;

  void * get_hp_raw_ptr(const std::string &key) override;

  common::runtime_type get_runtime_type() const override;
  std::string debug_str() const override;

  static inline std::shared_ptr<model>
  new_instance()
  {
    return std::make_shared<bbnc_model>();
  }

protected:
  float alpha_;
  float beta_;
};

} // namespace models
} // namespace microscopes
