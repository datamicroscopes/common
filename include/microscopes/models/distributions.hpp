#pragma once

#include <stdexcept>
#include <memory>

#include <microscopes/models/base.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/util.hpp>

#include <distributions/io/protobuf.hpp>
#include <distributions/models/bb.hpp>
#include <distributions/models/bnb.hpp>
#include <distributions/models/gp.hpp>
#include <distributions/models/nich.hpp>
#include <distributions/models/dd.hpp>

#define DISTRIB_BB_SHARED_FIELDS(x) \
  x(alpha) \
  x(beta)

#define DISTRIB_BB_GROUP_FIELDS(x) \
  x(heads) \
  x(tails)

#define DISTRIB_BNB_SHARED_FIELDS(x) \
  x(alpha) \
  x(beta) \
  x(r)

#define DISTRIB_BNB_GROUP_FIELDS(x) \
  x(count) \
  x(sum)

#define DISTRIB_GP_SHARED_FIELDS(x) \
  x(alpha) \
  x(inv_beta)

#define DISTRIB_GP_GROUP_FIELDS(x) \
  x(count) \
  x(sum) \
  x(log_prod)

#define DISTRIB_NICH_SHARED_FIELDS(x) \
  x(mu) \
  x(kappa) \
  x(sigmasq) \
  x(nu)

#define DISTRIB_NICH_GROUP_FIELDS(x) \
  x(count) \
  x(mean) \
  x(count_times_variance)

#define DISTRIB_FOR_EACH_DISTRIBUTION(x) \
  x(BetaBernoulli) \
  x(BetaNegativeBinomial) \
  x(GammaPoisson) \
  x(NormalInverseChiSq) \
  x(DirichletDiscrete128)

// NOTE: DirichletDiscrete128 does not appear here since it is special
#define DISTRIB_FOR_EACH_DISTRIBUTION_WITH_SHARED_FIELDS(x) \
  x(BetaBernoulli, DISTRIB_BB_SHARED_FIELDS) \
  x(BetaNegativeBinomial, DISTRIB_BNB_SHARED_FIELDS) \
  x(GammaPoisson, DISTRIB_GP_SHARED_FIELDS) \
  x(NormalInverseChiSq, DISTRIB_NICH_SHARED_FIELDS)

#define DISTRIB_FOR_EACH_DISTRIBUTION_WITH_GROUP_FIELDS(x) \
  x(BetaBernoulli, DISTRIB_BB_GROUP_FIELDS) \
  x(BetaNegativeBinomial, DISTRIB_BNB_GROUP_FIELDS) \
  x(GammaPoisson, DISTRIB_GP_GROUP_FIELDS) \
  x(NormalInverseChiSq, DISTRIB_NICH_GROUP_FIELDS)

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

template <typename T> struct distribution_types {};

#define DISTRIB_SPECIALIZE_MODEL_TYPES(name) \
  template <> \
  struct distribution_types< distributions::name > \
  { \
    typedef distributions::protobuf::name ## _Shared shared_message_type; \
    typedef distributions::protobuf::name ## _Group group_message_type; \
  };

DISTRIB_FOR_EACH_DISTRIBUTION(DISTRIB_SPECIALIZE_MODEL_TYPES)

#undef DISTRIB_SPECIALIZE_MODEL_TYPES

template <typename T> struct distributions_shared_hp {};
template <typename T> struct distributions_group_ss {};

#define DISTRIB_SPECIALIZE_RAW_PTR(fname) \
  if (key == #fname) return common::value_mutator(&s.fname);

#define DISTRIB_SPECIALIZE_SHARED_HP(name, fields) \
  template <> \
  struct distributions_shared_hp< distributions::name > \
  { \
    static inline common::value_mutator \
    get(distributions::name::Shared &s, const std::string &key) \
    { \
      fields(DISTRIB_SPECIALIZE_RAW_PTR) \
      throw std::runtime_error("Unknown shared HP param key: " + key); \
    } \
  };

#define DISTRIB_SPECIALIZE_GROUP_SS(name, fields) \
  template <> \
  struct distributions_group_ss< distributions::name > \
  { \
    static inline common::value_mutator \
    get(distributions::name::Group &s, const std::string &key) \
    { \
      fields(DISTRIB_SPECIALIZE_RAW_PTR) \
      throw std::runtime_error("Unknown group SS param key: " + key); \
    } \
  };

DISTRIB_FOR_EACH_DISTRIBUTION_WITH_SHARED_FIELDS(DISTRIB_SPECIALIZE_SHARED_HP)
DISTRIB_FOR_EACH_DISTRIBUTION_WITH_GROUP_FIELDS(DISTRIB_SPECIALIZE_GROUP_SS)

#undef DISTRIB_SPECIALIZE_RAW_PTR
#undef DISTRIB_SPECIALIZE_SHARED_HP
#undef DISTRIB_SPECIALIZE_GROUP_SS

// NOTE: DirichletDiscrete128 is special
template <>
struct distributions_shared_hp< distributions::DirichletDiscrete128 >
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
    throw std::runtime_error("Unknown shared HP param key: " + key);
  }
};

template <>
struct distributions_group_ss< distributions::DirichletDiscrete128 >
{
  static inline common::value_mutator
  get(distributions::DirichletDiscrete128::Group &s, const std::string &key)
  {
    if (key == "count_sum")
      return common::value_mutator(&s.count_sum);
    else if (key == "counts")
      return common::value_mutator(
          reinterpret_cast<uint8_t *>(&s.counts[0]),
          common::runtime_type(
            common::static_type_to_primitive_type<
              std::remove_reference<decltype(s.counts[0])>::type
            >::value,
            s.dim));
    throw std::runtime_error("Unknown group SS param key: " + key);
  }
};

template <typename T>
class distributions_group : public group {
private:

  static inline const typename T::Shared &
  shared_repr(const hypers &h);

public:
  typedef typename distribution_types<T>::group_message_type message_type;

  void
  add_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    MICROSCOPES_ASSERT(!value.ismasked(0));
    repr_.add_value(shared_repr(m), value.get< typename T::Value >(0), rng);
  }

  void
  remove_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    MICROSCOPES_ASSERT(!value.ismasked(0));
    repr_.remove_value(shared_repr(m), value.get< typename T::Value >(0), rng);
  }

  float
  score_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) const override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    MICROSCOPES_ASSERT(!value.ismasked(0));
    return repr_.score_value(shared_repr(m), value.get< typename T::Value >(0), rng);
  }

  float
  score_data(const hypers &m, common::rng_t &rng) const override
  {
    return repr_.score_data(shared_repr(m), rng);
  }

  void
  sample_value(const hypers &m, common::value_mutator &value, common::rng_t &rng) const override
  {
    MICROSCOPES_ASSERT(value.shape() == 1);
    typename T::Value sampled = repr_.sample_value(shared_repr(m), rng);
    value.set< typename T::Value >(sampled, 0);
  }

  common::suffstats_bag_t
  get_ss() const override
  {
    message_type m;
    repr_.protobuf_dump(m);
    return common::util::protobuf_to_string(m);
  }

  void
  set_ss(const common::suffstats_bag_t &ss) override
  {
    message_type m;
    common::util::protobuf_from_string(m, ss);
    repr_.protobuf_load(m);
  }

  void
  set_ss(const group &g) override
  {
    repr_ = static_cast<const distributions_group<T> &>(g).repr_;
  }

  common::value_mutator
  get_ss_mutator(const std::string &name) override
  {
    return distributions_group_ss<T>::get(repr_, name);
  }

  std::string
  debug_str() const override
  {
    // XXX: inefficient
    message_type m;
    repr_.protobuf_dump(m);
    return m.ShortDebugString();
  }

  typename T::Group repr_;
};

namespace detail {

template <typename T>
class distributions_hypers : public hypers {
public:
  typedef typename distribution_types<T>::shared_message_type message_type;

  std::shared_ptr<group>
  create_group(common::rng_t &rng) const override
  {
    auto p = std::make_shared<distributions_group<T>>();
    p->repr_.init(repr_, rng);
    return p;
  }

  common::hyperparam_bag_t
  get_hp() const override
  {
    message_type m;
    repr_.protobuf_dump(m);
    return common::util::protobuf_to_string(m);
  }

  void
  set_hp(const common::hyperparam_bag_t &hp) override
  {
    message_type m;
    common::util::protobuf_from_string(m, hp);
    repr_.protobuf_load(m);
  }

  void
  set_hp(const hypers &m) override
  {
    repr_ = static_cast<const distributions_hypers<T> &>(m).repr_;
  }

  common::value_mutator
  get_hp_mutator(const std::string &name) override
  {
    return distributions_shared_hp<T>::get(repr_, name);
  }

  std::string
  debug_str() const override
  {
    // XXX: inefficient
    message_type m;
    repr_.protobuf_dump(m);
    return m.ShortDebugString();
  }

  typename T::Shared repr_;
};

template <typename T>
class distributions_model : public model {
public:
  common::runtime_type
  get_runtime_type() const override
  {
    return common::runtime_type(
        common::static_type_to_primitive_type< typename T::Value >::value);
  }
};

// shorten name
typedef distributions::DirichletDiscrete128 DD128;

} // namespace detail

template <typename T>
class distributions_hypers : public detail::distributions_hypers<T> {
public:
  typedef
    typename detail::distributions_hypers<T>::message_type
    message_type;
};

template <>
class distributions_hypers<detail::DD128>
  : public detail::distributions_hypers<detail::DD128> {
public:
  typedef
    typename detail::distributions_hypers<detail::DD128>::message_type
    message_type;

  distributions_hypers(unsigned size)
  {
    MICROSCOPES_DCHECK(size > 0, "no elements");
    this->repr_.dim = size;
  }

  void
  set_hp(const common::hyperparam_bag_t &hp) override
  {
    message_type m;
    common::util::protobuf_from_string(m, hp);
    MICROSCOPES_DCHECK(this->repr_.dim == m.alphas_size(), "wrong dimension");
    this->repr_.protobuf_load(m);
  }

  void
  set_hp(const hypers &m) override
  {
    const auto &that = static_cast<const distributions_hypers<detail::DD128> &>(m);
    MICROSCOPES_DCHECK(this->repr_.dim == that.repr_.dim, "wrong dimension");
    this->repr_ = that.repr_;
  }
};

template <typename T>
class distributions_model : public detail::distributions_model<T> {
public:
  std::shared_ptr<hypers>
  create_hypers() const override
  {
    return std::make_shared<distributions_hypers<T>>();
  }
};

template <>
class distributions_model<detail::DD128>
  : public detail::distributions_model<detail::DD128> {
public:
  distributions_model(unsigned dim)
    : dim_(dim)
  {
    MICROSCOPES_DCHECK(dim > 0, "no elements");
  }

  std::shared_ptr<hypers>
  create_hypers() const override
  {
    return std::make_shared<distributions_hypers<detail::DD128>>(dim_);
  }

  common::runtime_type
  get_runtime_type() const override
  {
    return common::runtime_type(TYPE_I32);
  }

private:
  unsigned dim_;
};

// for cython
typedef distributions_model<detail::DD128> distributions_model_dd128;

// explicitly instantiate C++ templates
#define DISTRIB_EXPLICIT_INSTANTIATE(name) \
  extern template class distributions_group< distributions::name >; \
  extern template class distributions_hypers< distributions::name >; \
  extern template class distributions_model< distributions::name >;
DISTRIB_FOR_EACH_DISTRIBUTION(DISTRIB_EXPLICIT_INSTANTIATE)
#undef DISTRIB_EXPLICIT_INSTANTIATE

template <typename T>
inline const typename T::Shared &
distributions_group<T>::shared_repr(const hypers &h)
{
  return static_cast<const distributions_hypers<T> &>(h).repr_;
}

} // namespace models
} // namespace microscopes
