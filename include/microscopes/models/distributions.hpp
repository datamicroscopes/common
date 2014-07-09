#pragma once

#include <stdexcept>

#include <microscopes/models/base.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/util.hpp>

#include <distributions/io/protobuf.hpp>
#include <distributions/models/bb.hpp>
#include <distributions/models/bnb.hpp>
#include <distributions/models/gp.hpp>
#include <distributions/models/nich.hpp>
#include <distributions/models/dd.hpp>

// XXX: make sure to include the distribution above
#define DISTRIB_BB_FIELDS(x) \
  x(alpha) \
  x(beta)

#define DISTRIB_BNB_FIELDS(x) \
  x(alpha) \
  x(beta) \
  x(r)

#define DISTRIB_GP_FIELDS(x) \
  x(alpha) \
  x(inv_beta)

#define DISTRIB_NICH_FIELDS(x) \
  x(mu) \
  x(kappa) \
  x(sigmasq) \
  x(nu)

#define DISTRIB_FOR_EACH_DISTRIBUTION(x) \
  x(BetaBernoulli) \
  x(BetaNegativeBinomial) \
  x(GammaPoisson) \
  x(NormalInverseChiSq) \
  x(DirichletDiscrete128)

// NOTE: DirichletDiscrete128 does not appear here since it is special
#define DISTRIB_FOR_EACH_DISTRIBUTION_WITH_FIELDS(x) \
  x(BetaBernoulli, DISTRIB_BB_FIELDS) \
  x(BetaNegativeBinomial, DISTRIB_BNB_FIELDS) \
  x(GammaPoisson, DISTRIB_GP_FIELDS) \
  x(NormalInverseChiSq, DISTRIB_NICH_FIELDS)

// somewhat of a hack
namespace distributions {
  extern template struct DirichletDiscrete<128>;
  typedef DirichletDiscrete<128> DirichletDiscrete128;
  namespace protobuf {
    typedef DirichletDiscrete_Shared DirichletDiscrete128_Shared;
    typedef DirichletDiscrete_Group DirichletDiscrete128_Group;
  } // namespace protobuf
} // namespace distributions

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

template <typename T> struct distributions_model_hp {};

#define DISTRIB_SPECIALIZE_HP_RAW_PTR(fname) \
  if (key == #fname) return common::value_mutator(&s.fname);

#define DISTRIB_SPECIALIZE_HP(name, fields) \
  template <> \
  struct distributions_model_hp< distributions::name > \
  { \
    static inline common::value_mutator \
    get(distributions::name::Shared &s, const std::string &key) \
    { \
      fields(DISTRIB_SPECIALIZE_HP_RAW_PTR) \
      throw std::runtime_error("Unknown HP param key: " + key); \
    } \
  };

DISTRIB_FOR_EACH_DISTRIBUTION_WITH_FIELDS(DISTRIB_SPECIALIZE_HP)

#undef DISTRIB_SPECIALIZE_HP_RAW_PTR
#undef DISTRIB_SPECIALIZE_HP

// NOTE: DirichletDiscrete128 is special
template <>
struct distributions_model_hp< distributions::DirichletDiscrete128 >
{
  static inline common::value_mutator
  get(distributions::DirichletDiscrete128::Shared &s, const std::string &key)
  {
    if (key == "alphas")
      return common::value_mutator(
          reinterpret_cast<uint8_t *>(&s.alphas[0]),
          common::runtime_type(
            common::static_type_to_primitive_type<
              std::remove_reference<decltype(s.alphas[0])>::type
            >::value,
            s.dim));
    throw std::runtime_error("Unknown HP param key: " + key);
  }
};

template <typename T>
class distributions_model : public model {
public:
  typedef typename distributions_model_types<T>::shared_message_type message_type;

  inline std::shared_ptr<feature_group> create_feature_group(common::rng_t &rng) const override;

  inline common::hyperparam_bag_t
  get_hp() const override
  {
    message_type m;
    repr_.protobuf_dump(m);
    std::ostringstream out;
    m.SerializeToOstream(&out);
    return out.str();
  }

  inline void
  set_hp(const common::hyperparam_bag_t &hp) override
  {
    std::istringstream inp(hp);
    message_type m;
    m.ParseFromIstream(&inp);
    repr_.protobuf_load(m);
  }

  inline void
  set_hp(const model &m) override
  {
    repr_ = static_cast<const distributions_model<T> &>(m).repr_;
  }

  inline common::value_mutator
  get_hp_mutator(const std::string &name) override
  {
    return distributions_model_hp<T>::get(repr_, name);
  }

  inline common::runtime_type
  get_runtime_type() const override
  {
    return common::runtime_type(
        common::static_type_to_primitive_type< typename T::Value >::value);
  }

  inline std::string
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

  inline void
  add_value(const model &m, const common::value_accessor &value, common::rng_t &rng) override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    MICROSCOPES_ASSERT(!value.ismasked(0));
    repr_.add_value(shared_repr(m), value.get< typename T::Value >(0), rng);
  }

  inline void
  remove_value(const model &m, const common::value_accessor &value, common::rng_t &rng) override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    MICROSCOPES_ASSERT(!value.ismasked(0));
    repr_.remove_value(shared_repr(m), value.get< typename T::Value >(0), rng);
  }

  inline float
  score_value(const model &m, const common::value_accessor &value, common::rng_t &rng) const override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    MICROSCOPES_ASSERT(!value.ismasked(0));
    return repr_.score_value(shared_repr(m), value.get< typename T::Value >(0), rng);
  }

  inline float
  score_data(const model &m, common::rng_t &rng) const override
  {
    return repr_.score_data(shared_repr(m), rng);
  }

  inline void
  sample_value(const model &m, common::value_mutator &value, common::rng_t &rng) const override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    typename T::Value sampled = repr_.sample_value(shared_repr(m), rng);
    value.set< typename T::Value >(sampled, 0);
  }

  inline common::suffstats_bag_t
  get_ss() const override
  {
    message_type m;
    repr_.protobuf_dump(m);
    std::ostringstream out;
    m.SerializeToOstream(&out);
    return out.str();
  }

  inline void
  set_ss(const common::suffstats_bag_t &ss) override
  {
    std::istringstream inp(ss);
    message_type m;
    m.ParseFromIstream(&inp);
    repr_.protobuf_load(m);
  }

  inline common::value_mutator
  get_ss_mutator(const std::string &name) override
  {
    // XXX: implement me
    throw std::runtime_error("unsupported");
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
