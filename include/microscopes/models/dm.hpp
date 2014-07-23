#pragma once

#include <microscopes/common/assert.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/util.hpp>
#include <microscopes/models/base.hpp>

#include <distributions/io/protobuf.hpp>
#include <vector>

/**
 * The Dirichlet-Multinomial distribution
 */
namespace microscopes {
namespace models {

class dm_group : public group {
public:
  typedef distributions::protobuf::DirichletDiscrete_Group message_type;

  dm_group(unsigned categories)
    : counts_(categories) {}

  void
  add_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override
  {
    MICROSCOPES_ASSERT(value.shape() == categories());
    for (size_t i = 0; i < categories(); i++) {
      const unsigned ni = value.get<unsigned>(i);
      counts_[i] += ni;
    }
  }

  void
  remove_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) override
  {
    MICROSCOPES_ASSERT(value.shape() == categories());
    for (size_t i = 0; i < categories(); i++) {
      const unsigned ni = value.get<unsigned>(i);
      MICROSCOPES_ASSERT(counts_[i] >= ni);
      counts_[i] -= ni;
    }
  }

  float score_value(const hypers &m, const common::value_accessor &value, common::rng_t &rng) const override;
  float score_data(const hypers &m, common::rng_t &rng) const override;
  void sample_value(const hypers &m, common::value_mutator &value, common::rng_t &rng) const override;

  common::suffstats_bag_t
  get_ss() const override
  {
    message_type m;
    for (auto c : counts_)
      m.add_counts(c);
    return common::util::protobuf_to_string(m);
  }

  void
  set_ss(const common::suffstats_bag_t &ss) override
  {
    message_type m;
    common::util::protobuf_from_string(m, ss);
    MICROSCOPES_DCHECK(
        (size_t)m.counts_size() == categories(),
        "# categories mismatch");
    for (size_t i = 0; i < categories(); i++)
      counts_[i] = m.counts(i);
  }

  void
  set_ss(const group &g) override
  {
    const auto &h = static_cast<const dm_group &>(g);
    MICROSCOPES_DCHECK(categories() == h.categories(),
        "# categories mismatch");
    *this = h;
  }

  common::value_mutator
  get_ss_mutator(const std::string &key) override
  {
    if (key == "counts")
      return common::value_mutator(
          reinterpret_cast<uint8_t *>(&counts_[0]),
          common::runtime_type(
            common::static_type_to_primitive_type<unsigned>::value,
            categories()));
    throw std::runtime_error("unknown key: " + key);
  }

  std::string
  debug_str() const override
  {
    std::ostringstream oss;
    oss << counts_;
    return oss.str();
  }

  inline size_t
  categories() const
  {
    return counts_.size();
  }

private:
  std::vector<unsigned> counts_;
};

class dm_hypers : public hypers {
public:
  typedef distributions::protobuf::DirichletDiscrete_Shared message_type;

  dm_hypers(unsigned categories)
    : alphas_(categories) {}

  std::shared_ptr<group>
  create_group(common::rng_t &rng) const override
  {
    return std::make_shared<dm_group>(categories());
  }

  common::hyperparam_bag_t
  get_hp() const override
  {
    message_type m;
    for (auto a : alphas_)
      m.add_alphas(a);
    return common::util::protobuf_to_string(m);
  }

  void
  set_hp(const common::hyperparam_bag_t &hp) override
  {
    message_type m;
    common::util::protobuf_from_string(m, hp);
    MICROSCOPES_DCHECK(
        (size_t)m.alphas_size() == categories(),
        "# categories mismatch");
    for (size_t i = 0; i < categories(); i++) {
      MICROSCOPES_DCHECK(m.alphas(i) > 0.,
          "alphas need to be positive reals");
      alphas_[i] = m.alphas(i);
    }
  }

  void
  set_hp(const hypers &m) override
  {
    const auto &h = static_cast<const dm_hypers &>(m);
    MICROSCOPES_DCHECK(categories() == h.categories(),
        "# categories mismatch");
    *this = h;
  }

  common::value_mutator
  get_hp_mutator(const std::string &key) override
  {
    if (key == "alphas")
      return common::value_mutator(
          reinterpret_cast<uint8_t *>(&alphas_[0]),
          common::runtime_type(
            common::static_type_to_primitive_type<float>::value,
            categories()));
    throw std::runtime_error("unknown key: " + key);
  }

  inline size_t
  categories() const
  {
    return alphas_.size();
  }

  inline const std::vector<float> &
  alphas() const
  {
    return alphas_;
  }

  std::string
  debug_str() const override
  {
    std::ostringstream oss;
    oss << alphas_;
    return oss.str();
  }

private:
  std::vector<float> alphas_;
};

class dm_model : public model {
public:
  dm_model(unsigned categories)
    : categories_(categories)
  {
    MICROSCOPES_DCHECK(categories > 0, "empty categories");
  }

  std::shared_ptr<hypers>
  create_hypers() const override
  {
    return std::make_shared<dm_hypers>(categories_);
  }

  common::runtime_type
  get_runtime_type() const override
  {
    return common::runtime_type(TYPE_I32, categories());
  }

  inline size_t
  categories() const
  {
    return categories_;
  }

private:
  unsigned categories_;
};

} // namespace models
} // namespace microscopes
