#pragma once

#include <microscopes/common/assert.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/models/base.hpp>
#include <eigen3/Eigen/Dense>

namespace microscopes {
namespace models {

class niw_hypers; // forward decl

class niw_group : public group {
  friend class niw_hypers;
public:
  niw_group(unsigned dim)
    : count_(),
      sum_x_(Eigen::VectorXf::Zero(dim)),
      sum_xxT_(Eigen::MatrixXf::Zero(dim, dim))
  {
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

  struct suffstats_t {
    Eigen::VectorXf mu0_;
    float lambda_;
    Eigen::MatrixXf psi_;
    float nu_;
    suffstats_t() : mu0_(), lambda_(), psi_(), nu_() {}
  };

  void postParams(const niw_hypers &m, suffstats_t &ss) const;

  inline size_t
  dim() const
  {
    return sum_x_.size();
  }

  unsigned count_;
  Eigen::VectorXf sum_x_;
  Eigen::MatrixXf sum_xxT_;
};

class niw_hypers : public hypers {
  friend class niw_group;
public:
  niw_hypers(unsigned dim)
    : mu0_(Eigen::VectorXf::Zero(dim)),
      lambda_(),
      psi_(Eigen::MatrixXf::Identity(dim, dim)),
      nu_() {}

  std::shared_ptr<group> create_group(common::rng_t &rng) const override;

  common::hyperparam_bag_t get_hp() const override;
  void set_hp(const common::hyperparam_bag_t &hp) override;
  void set_hp(const hypers &m) override;

  common::value_mutator get_hp_mutator(const std::string &key) override;

  std::string debug_str() const override;

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

class niw_model : public model {
public:
  niw_model(unsigned dim)
    : dim_(dim)
  {
    MICROSCOPES_DCHECK(dim > 0, "empty dimension");
  }

  std::shared_ptr<hypers>
  create_hypers() const override
  {
    return std::make_shared<niw_hypers>(dim_);
  }

  common::runtime_type get_runtime_type() const override;

  inline size_t
  dim() const
  {
    return dim_;
  }

private:
  unsigned dim_;
};

} // namespace models
} // namespace microscopes
