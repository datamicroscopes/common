#pragma once

#include <microscopes/models/base.hpp>
#include <microscopes/common/util.hpp>

#include <distributions/io/protobuf.hpp>
#include <distributions/models/bb.hpp>
#include <distributions/models/bnb.hpp>
#include <distributions/models/gp.hpp>
#include <distributions/models/nich.hpp>

#include <stdexcept>

// XXX: make sure to include the distribution above
#define DISTRIB_FOR_EACH_DISTRIBUTION(x) \
  x(BetaBernoulli) \
  x(BetaNegativeBinomial) \
  x(GammaPoisson) \
  x(NormalInverseChiSq)

namespace microscopes {
namespace models {

template <typename T> struct distributions_model_types {};

#define DISTRIB_SPECIALIZE_MODEL_TYPES(name) \
  template <> \
  struct distributions_model_types< distributions::name > \
  { \
    typedef distributions::protobuf::name ## _Shared shared_message_type; \
    typedef distributions::protobuf::name ## _Group group_message_type; \
  };

DISTRIB_FOR_EACH_DISTRIBUTION(DISTRIB_SPECIALIZE_MODEL_TYPES)

#undef DISTRIB_SPECIALIZE_MODEL_TYPES

// hardcode here which distributions models contain only scalar float parameters
template <typename T>
struct distributions_model_hp_float_bag
{
  static inline common::hyperparam_float_bag_t
  get(const typename T::Shared &)
  {
    throw std::runtime_error("this model does not support float bags");
  }
};

#define DISTRIB_BB_FIELDS(x) \
  x(alpha) \
  x(beta)

#define DISTRIB_GP_FIELDS(x) \
  x(alpha) \
  x(inv_beta)

#define DISTRIB_NICH_FIELDS(x) \
  x(mu) \
  x(kappa) \
  x(sigmasq) \
  x(nu)

#define DISTRIB_FOR_EACH_FLOAT_DISTRIBUTION(x) \
  x(BetaBernoulli, DISTRIB_BB_FIELDS) \
  x(GammaPoisson, DISTRIB_GP_FIELDS) \
  x(NormalInverseChiSq, DISTRIB_NICH_FIELDS)

// XXX: yes hardcoding sucks, and we could do this by introspection/reflection
// on the protobuf message, but that is annoying and this is faster. deal with it.

#define DISTRIB_SPECIALIZE_FLOAT_BAG_EMIT_ASSIGN(fname) \
  ret[ #fname ] = s.fname;

#define DISTRIB_SPECIALIZE_FLOAT_BAG(name, fields) \
  template <> \
  struct distributions_model_hp_float_bag< distributions::name > \
  { \
    static inline common::hyperparam_float_bag_t \
    get(const distributions::name::Shared &s) \
    { \
      common::hyperparam_float_bag_t ret; \
      fields(DISTRIB_SPECIALIZE_FLOAT_BAG_EMIT_ASSIGN) \
      return ret; \
    } \
  };

DISTRIB_FOR_EACH_FLOAT_DISTRIBUTION(DISTRIB_SPECIALIZE_FLOAT_BAG)

#undef DISTRIB_SPECIALIZE_FLOAT_BAG_EMIT_ASSIGN
#undef DISTRIB_SPECIALIZE_FLOAT_BAG

template <typename T>
class distributions_model : public model {
public:
  typedef typename distributions_model_types<T>::shared_message_type message_type;

  std::shared_ptr<feature_group> create_feature_group(common::rng_t &rng) const override;

  common::hyperparam_bag_t
  get_hp() const override
  {
    message_type m;
    repr_.protobuf_dump(m);
    std::ostringstream out;
    m.SerializeToOstream(&out);
    return out.str();
  }

  common::hyperparam_float_bag_t
  get_hp_float_bag() const override
  {
    return distributions_model_hp_float_bag<T>::get(repr_);
  }

  void
  set_hp(const common::hyperparam_bag_t &hp) override
  {
    std::istringstream inp(hp);
    message_type m;
    m.ParseFromIstream(&inp);
    repr_.protobuf_load(m);
  }

  void
  set_hp(const model &m) override
  {
    repr_ = static_cast<const distributions_model<T> &>(m).repr_;
  }

  runtime_type_info
  get_runtime_type_info() const override
  {
    return common::_static_type_to_runtime_id< typename T::Value >::value;
  }

  std::string
  debug_str() const override
  {
    // XXX: inefficient
    message_type m;
    repr_.protobuf_dump(m);
    return m.ShortDebugString();
  }

  // XXX: public for now so distributions_feature_group can access
  typename T::Shared repr_;
};

template <typename T>
class distributions_feature_group : public feature_group {
private:

  static inline const typename T::Shared &
  shared_repr(const model &m)
  {
    return static_cast<const distributions_model<T> &>(m).repr_;
  }

public:
  typedef typename distributions_model_types<T>::group_message_type message_type;

  void
  add_value(const model &m, const common::row_accessor &value, common::rng_t &rng) override
  {
    repr_.add_value(shared_repr(m), value.get< typename T::Value >(), rng);
  }

  void
  remove_value(const model &m, const common::row_accessor &value, common::rng_t &rng) override
  {
    repr_.remove_value(shared_repr(m), value.get< typename T::Value >(), rng);
  }

  float
  score_value(const model &m, const common::row_accessor &value, common::rng_t &rng) const override
  {
    return repr_.score_value(shared_repr(m), value.get< typename T::Value >(), rng);
  }

  float
  score_data(const model &m, common::rng_t &rng) const override
  {
    return repr_.score_data(shared_repr(m), rng);
  }

  void
  sample_value(const model &m, common::row_mutator &value, common::rng_t &rng) const override
  {
    typename T::Value sampled = repr_.sample_value(shared_repr(m), rng);
    value.set< typename T::Value >(sampled);
  }

  common::suffstats_bag_t
  get_ss() const override
  {
    message_type m;
    repr_.protobuf_dump(m);
    std::ostringstream out;
    m.SerializeToOstream(&out);
    return out.str();
  }

  void
  set_ss(const common::suffstats_bag_t &ss) override
  {
    std::istringstream inp(ss);
    message_type m;
    m.ParseFromIstream(&inp);
    repr_.protobuf_load(m);
  }

  // XXX: public for now so distributions_model can access
  typename T::Group repr_;
};

template <typename T>
std::shared_ptr<feature_group>
distributions_model<T>::create_feature_group(common::rng_t &rng) const
{
  auto p = std::make_shared<distributions_feature_group<T>>();
  p->repr_.init(repr_, rng);
  return p;
}

// makes it easier for python code to construct models
template <typename T>
struct distributions_factory {
  inline std::shared_ptr<model>
  new_instance() const
  {
    return std::make_shared<distributions_model<T>>();
  }
};

// explicitly instantiate C++ templates
#define DISTRIB_EXPLICIT_INSTANTIATE(name) \
  extern template class distributions_model< distributions::name >;
DISTRIB_FOR_EACH_DISTRIBUTION(DISTRIB_EXPLICIT_INSTANTIATE)
#undef DISTRIB_EXPLICIT_INSTANTIATE

} // namespace models
} // namespace microscopes
