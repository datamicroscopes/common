#pragma once

#include <microscopes/common/assert.hpp>
#include <microscopes/models/base.hpp>
#include <eigen3/Eigen/Dense>

namespace microscopes {
namespace models {

class niw_model; // forward decl

class niw_feature_group : public feature_group {
  friend class niw_model;
public:
  niw_feature_group(unsigned dim)
    : count_(),
      sum_x_(Eigen::VectorXf::Zero(dim)),
      sum_xxT_(Eigen::MatrixXf::Zero(dim, dim))
  {
  }

  void add_value(const model &m, const common::value_accessor &value, common::rng_t &rng) override;
  void remove_value(const model &m, const common::value_accessor &value, common::rng_t &rng) override;
  float score_value(const model &m, const common::value_accessor &value, common::rng_t &rng) const override;
  float score_data(const model &m, common::rng_t &rng) const override;
  void sample_value(const model &m, common::value_mutator &value, common::rng_t &rng) const override;

  common::suffstats_bag_t get_ss() const override;
  void set_ss(const common::suffstats_bag_t &ss) override;

  void * get_ss_raw_ptr(const std::string &key) override;

protected:

  struct suffstats_t {
    Eigen::VectorXf mu0_;
    float lambda_;
    Eigen::MatrixXf psi_;
    float nu_;
    suffstats_t() : mu0_(), lambda_(), psi_(), nu_() {}
  };

  void postParams(const niw_model &m, suffstats_t &ss) const;

  inline size_t
  dim() const
  {
    return sum_x_.size();
  }

  unsigned count_;
  Eigen::VectorXf sum_x_;
  Eigen::MatrixXf sum_xxT_;
};

class niw_model : public model {
  friend class niw_feature_group;
public:
  niw_model() : mu0_(), lambda_(), psi_(), nu_() {}

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
    return std::make_shared<niw_model>();
  }

  inline size_t
  dim() const
  {
    return mu0_.size();
  }

protected:
  Eigen::VectorXf mu0_;
  float lambda_;
  Eigen::MatrixXf psi_;
  float nu_;
};

} // namespace models
} // namespace microscopes

