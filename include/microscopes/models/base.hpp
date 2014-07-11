#pragma once

#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/random_fwd.hpp>
#include <microscopes/common/typedefs.hpp>

#include <memory>

namespace microscopes {
namespace models {

// forward decl
class model;

// abstract feature group
class feature_group {
public:
  virtual ~feature_group() {}

  virtual void add_value(const model &m, const common::value_accessor &value, common::rng_t &rng) = 0;
  virtual void remove_value(const model &m, const common::value_accessor &value, common::rng_t &rng) = 0;
  virtual float score_value(const model &m, const common::value_accessor &value, common::rng_t &rng) const = 0;
  virtual float score_data(const model &m, common::rng_t &rng) const = 0;
  virtual void sample_value(const model &m, common::value_mutator &value, common::rng_t &rng) const = 0;

  virtual common::suffstats_bag_t get_ss() const = 0;
  virtual void set_ss(const common::suffstats_bag_t &ss) = 0;

  virtual common::value_mutator get_ss_mutator(const std::string &key) = 0;
  virtual std::string debug_str() const = 0;
};

// abstract model
class model {
public:
  virtual ~model() {}

  virtual std::shared_ptr<feature_group> create_feature_group(common::rng_t &rng) const = 0;

  virtual common::hyperparam_bag_t get_hp() const = 0;
  virtual void set_hp(const common::hyperparam_bag_t &hp) = 0;
  virtual void set_hp(const model &m) = 0;

  virtual common::value_mutator get_hp_mutator(const std::string &key) = 0;

  virtual common::runtime_type get_runtime_type() const = 0;
  virtual std::string debug_str() const = 0;
};

// useful typedefs for cython
typedef model* model_raw_ptr;
typedef std::shared_ptr<model> model_shared_ptr;

} // namespace models
} // namespace microscopes
