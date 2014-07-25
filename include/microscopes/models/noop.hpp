#pragma once

#include <microscopes/models/base.hpp>

namespace microscopes {
namespace models {

/**
 * A stub model, used to measure the function call overhead of the likelihood
 * model API
 */

class noop_group : public group {
public:
  void add_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override {}
  void remove_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override {}
  float score_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) const override { return 0.0; }
  float score_data(const hypers &m, common::rng_t &rng) const override { return 0.0; }
  void sample_value(const hypers &m, common::value_mutator &value, common::rng_t &rng) const override {}
  common::suffstats_bag_t get_ss() const override { return ""; }
  void set_ss(const common::suffstats_bag_t &ss) override {}
  void set_ss(const group &g) override {}
  common::value_mutator get_ss_mutator(const std::string &key) override {
    throw std::runtime_error("noop");
  }
  std::string debug_str() const override { return "<noop>"; }
};

class noop_hypers : public hypers {
public:
  common::hyperparam_bag_t get_hp() const override { return ""; }
  void set_hp(const common::hyperparam_bag_t &hp) override {}
  void set_hp(const hypers &s) override {}
  common::value_mutator get_hp_mutator(const std::string &key) override {
    throw std::runtime_error("noop");
  }
  std::shared_ptr<group> create_group(common::rng_t &rng) const override {
    return std::make_shared<noop_group>();
  }
  std::string debug_str() const override { return "<noop>"; }
};

class noop_model : public model {
public:
  std::shared_ptr<hypers> create_hypers() const override {
    return std::make_shared<noop_hypers>();
  }
  // since we have no void type, we arbitrarily pick on
  common::runtime_type get_runtime_type() const override {
    return common::runtime_type(TYPE_B);
  }
};

} // namespace models
} // namespace microscopes
