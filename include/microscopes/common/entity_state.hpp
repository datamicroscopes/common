#pragma once

#include <microscopes/common/random_fwd.hpp>
#include <microscopes/common/runtime_type.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/typedefs.hpp>
#include <microscopes/models/base.hpp>

namespace microscopes {
namespace common {

class fixed_entity_based_state_object {
public:
  virtual ~fixed_entity_based_state_object() {}

  virtual size_t nentities() const = 0;
  virtual size_t ngroups() const = 0;
  virtual size_t ncomponents() const = 0;

  virtual std::vector<ssize_t> assignments() const = 0;
  virtual std::vector<size_t> groups() const = 0;

  virtual size_t groupsize(size_t gid) const = 0;

  // Routines for manipulating the parameters

  virtual hyperparam_bag_t get_cluster_hp() const = 0;
  virtual void set_cluster_hp(const hyperparam_bag_t &hp) = 0;
  virtual value_mutator get_cluster_hp_mutator(const std::string &key) = 0;

  virtual hyperparam_bag_t get_component_hp(size_t component) const = 0;
  virtual void set_component_hp(size_t component, const hyperparam_bag_t &hp) = 0;
  virtual void set_component_hp(size_t component, const models::hypers &proto) = 0;
  virtual value_mutator get_component_hp_mutator(size_t component, const std::string &key) = 0;

  virtual std::vector<ident_t> suffstats_identifiers(size_t component) const = 0;
  virtual suffstats_bag_t get_suffstats(size_t component, ident_t id) const = 0;
  virtual void set_suffstats(size_t component, ident_t id, const suffstats_bag_t &ss) = 0;
  virtual value_mutator get_suffstats_mutator(size_t component, ident_t id, const std::string &key) = 0;

  // Routines for manipulating cluster membership

  virtual void add_value(size_t gid, size_t eid, rng_t &rng) = 0;
  virtual size_t remove_value(size_t eid, rng_t &rng) = 0;

  virtual std::pair<std::vector<size_t>, std::vector<float>>
  score_value(size_t eid, rng_t &rng) const
  {
    // delegates to inplace_score_value by default
    std::pair<std::vector<size_t>, std::vector<float>> ret;
    inplace_score_value(ret, eid, rng);
    return ret;
  }

  virtual void
  inplace_score_value(std::pair<std::vector<size_t>, std::vector<float>> &scores,
                      size_t eid,
                      rng_t &rng) const = 0;

  virtual float score_assignment() const = 0;

  virtual float score_likelihood(size_t component, ident_t id, rng_t &rng) const = 0;

  virtual float
  score_likelihood(size_t component, rng_t &rng) const
  {
    float score = 0.;
    for (auto id : suffstats_identifiers(component))
      score += score_likelihood(component, id, rng);
    return score;
  }
};

/**
 * The common interface for sampling kernels:
 *
 * An entity_based_state_object is a collection of (a fixed, known number of)
 * entities.
 *
 * The state describes a generative process composed of:
 *   (A) the clustering (group assignment) of the entities
 *   (B) conditionally independent components which describe the likelihood model
 *
 * Examples of generative models falling under an entity_based_state_object
 * include (1) mixture models and (2) infinite relational models (IRM)
 */
class entity_based_state_object : public fixed_entity_based_state_object {
public:
  /**
   * A subset of groups()
   */
  virtual std::vector<size_t> empty_groups() const = 0;
  virtual size_t create_group(rng_t &rng) = 0;
  virtual void delete_group(size_t gid) = 0;
};

} // namespace common
} // namespace microscopes
