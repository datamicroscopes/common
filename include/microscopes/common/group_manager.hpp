#pragma once

#include <microscopes/common/typedefs.hpp>
#include <microscopes/common/macros.hpp>
#include <microscopes/common/assert.hpp>
#include <microscopes/common/util.hpp>
#include <microscopes/common/runtime_value.hpp>
#include <microscopes/io/schema.pb.h>
#include <distributions/special.hpp>
#include <distributions/io/protobuf.hpp>

#include <cmath>
#include <vector>
#include <set>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <stdexcept>

namespace microscopes {
namespace common {

// T needs default no-arg ctor
template <typename T>
struct gd {
  gd() : count_(), data_() {}
  size_t count_;
  T data_;
};

template <typename T>
class fixed_group_manager {
public:
  typedef distributions::protobuf::DirichletDiscrete_Shared message_type;
  typedef typename std::vector<std::pair<size_t, gd<T>>>::const_iterator const_iterator;

  fixed_group_manager(size_t n, size_t k)
    : alphas_(k),
      assignments_(n, -1),
      groups_(k)
  {
    for (size_t i = 0; i < k; i++)
      groups_[i].first = i;
  }

  inline hyperparam_bag_t
  get_hp() const
  {
    message_type m;
    for (auto a : alphas_)
      m.add_alphas(a);
    std::ostringstream out;
    m.SerializeToOstream(&out);
    return out.str();
  }

  inline void
  set_hp(const hyperparam_bag_t &hp)
  {
    std::istringstream inp(hp);
    message_type m;
    m.ParseFromIstream(&inp);
    MICROSCOPES_DCHECK(m.alphas_size() == alphas_.size(), "size doesn't match");
    for (size_t i = 0; i < alphas_.size(); i++) {
      MICROSCOPES_DCHECK(m.alphas(i) > 0., "alphas need to be positive");
      alphas_[i] = m.alphas(i);
    }
  }

  inline value_mutator
  get_hp_mutator(const std::string &key)
  {
    if (key == "alphas")
      return value_mutator(
          reinterpret_cast<uint8_t *>(&alphas_[0]),
          runtime_type(TYPE_F32, alphas_.size()));
    throw std::runtime_error("unknown key: " + key);
  }

  inline const std::vector<ssize_t> &
  assignments() const
  {
    return assignments_;
  }

  inline size_t nentities() const { return assignments_.size(); }
  inline size_t ngroups() const { return groups_.size(); }

  inline size_t
  groupsize(size_t gid) const
  {
    return group(gid).count_;
  }

  inline const gd<T> &
  group(size_t gid) const
  {
    MICROSCOPES_DCHECK(gid < ngroups(), "invalid gid");
    return groups_[gid].second;
  }

  inline gd<T> &
  group(size_t gid)
  {
    MICROSCOPES_DCHECK(gid < ngroups(), "invalid gid");
    return groups_[gid].second;
  }

  inline std::vector<size_t>
  groups() const
  {
    return util::range(ngroups());
  }

  inline const_iterator
  begin() const
  {
    return groups_.begin();
  }

  inline const_iterator
  end() const
  {
    return groups_.end();
  }

  inline T &
  add_value(size_t gid, size_t eid)
  {
    MICROSCOPES_DCHECK(gid < ngroups(), "invalid gid");
    MICROSCOPES_DCHECK(eid < nentities(), "invalid eid");
    MICROSCOPES_DCHECK(assignments_[eid] == -1, "entity already assigned");
    assignments_[eid] = gid;
    auto &g = groups_[gid].second;
    g.count_++;
    return g.data_;
  }

  inline std::pair<size_t, T&>
  remove_value(size_t eid)
  {
    MICROSCOPES_DCHECK(eid < nentities(), "invalid eid");
    MICROSCOPES_DCHECK(assignments_[eid] != -1, "entity not assigned");
    const size_t gid = assignments_[eid];
    MICROSCOPES_ASSERT(gid < ngroups());
    assignments_[eid] = -1;
    auto &g = groups_[gid].second;
    MICROSCOPES_ASSERT(g.count_);
    g.count_--;
    return std::pair<size_t, T&>(gid, g.data_);
  }

  inline float
  score_assignment() const
  {
    using distributions::fast_lgamma;

    float score = 0.;
    float alpha_sum = 0.;
    size_t count_sum = 0;

    for (size_t i = 0; i < ngroups(); i++) {
      const float alpha = alphas_[i];
      const size_t count = groups_[i].second.count_;
      alpha_sum += alpha;
      count_sum += count;
      score += fast_lgamma(alpha + float(count_sum)) - fast_lgamma(alpha);
    }

    score += fast_lgamma(alpha_sum)
      - fast_lgamma(alpha_sum + float(count_sum));
    return score;
  }

  inline float
  pseudocount(size_t gid, const gd<T> &g) const
  {
    MICROSCOPES_DCHECK(gid < ngroups(), "invalid gid");
    return alphas_[gid] + g.count_;
  }

private:
  std::vector<float> alphas_;
  std::vector<ssize_t> assignments_;
  std::vector<std::pair<size_t, gd<T>>> groups_;
};

template <typename T>
class group_manager {
public:
  typedef io::CRP message_type;
  typedef typename std::map<size_t, gd<T>>::const_iterator const_iterator;

  group_manager(size_t n)
    : alpha_(),
      gcount_(),
      gempty_(),
      assignments_(n, -1),
      groups_()
  {}

  inline hyperparam_bag_t
  get_hp() const
  {
    message_type m;
    m.set_alpha(alpha_);
    std::ostringstream out;
    m.SerializeToOstream(&out);
    return out.str();
  }

  inline void
  set_hp(const hyperparam_bag_t &hp)
  {
    std::istringstream inp(hp);
    message_type m;
    m.ParseFromIstream(&inp);
    MICROSCOPES_DCHECK(m.alpha() > 0.0, "alpha must be positive");
    alpha_ = m.alpha();
  }

  inline value_mutator
  get_hp_mutator(const std::string &key)
  {
    if (key == "alpha")
      return value_mutator(&alpha_);
    throw std::runtime_error("unknown key: " + key);
  }

  inline const std::vector<ssize_t> &
  assignments() const
  {
    return assignments_;
  }

  inline const std::set<size_t> &
  empty_groups() const
  {
    return gempty_;
  }

  inline size_t nentities() const { return assignments_.size(); }
  inline size_t ngroups() const { return groups_.size(); }

  inline bool
  isactivegroup(size_t gid) const
  {
    return groups_.find(gid) != groups_.end();
  }

  inline size_t
  groupsize(size_t gid) const
  {
    return group(gid).count_;
  }

  inline const gd<T> &
  group(size_t gid) const
  {
    const auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    return it->second;
  }

  inline gd<T> &
  group(size_t gid)
  {
    auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    return it->second;
  }

  inline std::vector<size_t>
  groups() const
  {
    std::vector<size_t> ret;
    ret.reserve(ngroups());
    for (auto &g : groups_)
      ret.push_back(g.first);
    return ret;
  }

  inline const_iterator
  begin() const
  {
    return groups_.begin();
  }

  inline const_iterator
  end() const
  {
    return groups_.end();
  }

  inline std::pair<size_t, T&>
  create_group()
  {
    const size_t gid = gcount_++;
    auto &g = groups_[gid]; // create the group
    MICROSCOPES_ASSERT(!gempty_.count(gid));
    gempty_.insert(gid);
    return std::pair<size_t, T&>(gid, g.data_);
  }

  inline void
  delete_group(size_t gid)
  {
    auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    MICROSCOPES_DCHECK(!it->second.count_, "group not empty");
    MICROSCOPES_ASSERT(gempty_.count(gid));
    groups_.erase(it);
    gempty_.erase(gid);
  }

  inline T &
  add_value(size_t gid, size_t eid)
  {
    MICROSCOPES_DCHECK(assignments_.at(eid) == -1, "entity already assigned");
    auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    if (!it->second.count_++) {
      MICROSCOPES_ASSERT(gempty_.count(gid));
      gempty_.erase(gid);
      MICROSCOPES_ASSERT(!gempty_.count(gid));
    } else {
      MICROSCOPES_ASSERT(!gempty_.count(gid));
    }
    assignments_[eid] = gid;
    return it->second.data_;
  }

  inline std::pair<size_t, T&>
  remove_value(size_t eid)
  {
    MICROSCOPES_DCHECK(assignments_.at(eid) != -1, "entity not assigned");
    const size_t gid = assignments_[eid];
    auto it = groups_.find(gid);
    MICROSCOPES_ASSERT(it != groups_.end());
    MICROSCOPES_ASSERT(!gempty_.count(gid));
    MICROSCOPES_ASSERT(it->second.count_);
    if (!--it->second.count_)
      gempty_.insert(gid);
    assignments_[eid] = -1;
    return std::pair<size_t, T&>(gid, it->second.data_);
  }

  inline float
  score_assignment() const
  {
    using distributions::fast_log;
    std::map<size_t, size_t> counts;
    MICROSCOPES_DCHECK(assignments_[0] != -1, "not assigned");
    counts[assignments_[0]] = 1;
    float sum = 0.;
    for (size_t i = 1; i < assignments_.size(); i++) {
      const ssize_t gid = assignments_[i];
      MICROSCOPES_DCHECK(gid != -1, "not assigned");
      const auto it = counts.find(gid);
      const bool found = (it != counts.end());
      const float numer = (!found) ? alpha_ : it->second;
      const float denom = float(i) + alpha_;
      sum += fast_log(numer / denom);
      if (found)
        it->second++;
      else
        counts[gid] = 1;
    }
    return sum;
  }

  inline float
  pseudocount(size_t gid, const gd<T> &g) const
  {
    if (g.count_)
      return g.count_;
    else {
      MICROSCOPES_ASSERT(gempty_.size());
      return alpha_ / float(gempty_.size());
    }
  }

protected:
  float alpha_;
  size_t gcount_;
  std::set<size_t> gempty_;
  std::vector<ssize_t> assignments_;
  std::map<size_t, gd<T>> groups_;
};

} // namespace common
} // namespace microscopes
