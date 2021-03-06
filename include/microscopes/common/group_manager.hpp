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
  gd(size_t count, const T& data) : count_(count), data_(data) {}
  gd(size_t count, T&& data) : count_(count), data_(std::move(data)) {}

  inline bool
  operator==(const gd &that) const
  {
    return count_ == that.count_ && data_ == that.data_;
  }

  inline bool
  operator!=(const gd &that) const
  {
    return !operator==(that);
  }

  size_t count_;
  T data_;
};

template <typename T>
class group_manager {
public:
  typedef io::CRP message_type;
  typedef typename std::map<size_t, gd<T>>::const_iterator const_iterator;

  // for std containers
  group_manager()
    : alpha_(),
      gcount_(),
      gempty_(),
      assignments_(),
      groups_()
  {}

  group_manager(size_t n)
    : alpha_(),
      gcount_(),
      gempty_(),
      assignments_(n, -1),
      groups_()
  {}

  group_manager(
      const serialized_t &repr,
      std::function<T(const std::string &)> group_deserializer_fn)
    : alpha_(), gcount_(), gempty_(), assignments_(), groups_()
  {
    io::GroupManager m;
    util::protobuf_from_string(m, repr);
    MICROSCOPES_DCHECK(m.alpha() > 0., "alphas can only be positive");
    MICROSCOPES_DCHECK(m.assignments_size() > 0, "no entities given");

    alpha_ = m.alpha();
    std::map<size_t, size_t> counts;
    for (size_t i = 0; i < (size_t)m.assignments_size(); i++) {
      MICROSCOPES_DCHECK(
          m.assignments(i) == -1 || m.assignments(i) >= 0,
          "invalid group id");
      assignments_.push_back(m.assignments(i));
      if (assignments_.back() != -1)
        counts[assignments_.back()]++;
    }

    for (size_t i = 0; i < (size_t)m.groups_size(); i++) {
      const auto &g = m.groups(i);
      const auto it = counts.find(g.id());
      const size_t count = (it == counts.end()) ? 0 : it->second;
      gd<T> gdata(count, std::move(group_deserializer_fn(g.data())));
      groups_[g.id()] = std::move(gdata);
      if (!count)
        gempty_.insert(g.id());
    }

    // gcount_ is 1+max group id seen
    if (!groups_.empty())
      gcount_ = groups_.crbegin()->first + 1;
  }

  inline hyperparam_bag_t
  get_hp() const
  {
    message_type m;
    m.set_alpha(alpha_);
    return util::protobuf_to_string(m);
  }

  inline void
  set_hp(const hyperparam_bag_t &hp)
  {
    message_type m;
    util::protobuf_from_string(m, hp);
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

  serialized_t
  serialize(std::function<serialized_t(const T &)> group_serializer_fn) const
  {
    io::GroupManager m;
    m.set_alpha(alpha_);
    for (auto s : assignments_)
      m.add_assignments(s);
    for (auto &p : groups_) {
      io::GroupData &g = *m.add_groups();
      g.set_id(p.first);
      g.set_data(group_serializer_fn(p.second.data_));
    }
    return util::protobuf_to_string(m);
  }

protected:
  float alpha_;
  size_t gcount_;
  std::set<size_t> gempty_;
  std::vector<ssize_t> assignments_;
  std::map<size_t, gd<T>> groups_;
};

/**
 * Manages groups and nothing else.
 *
 * XXX(stephentu):
 *   (a) group_manager should use this underneath
 *   (b) rename group_manager to CRP
 *
 * Let N = # of groups.
 *   (a) O(log N) - add/remove group
 *   (b) O(N) - iteration
 *   (c) O(1) - size
 */
template <typename T>
class simple_group_manager {
public:
  typedef typename std::map<size_t, T>::const_iterator const_iterator;

  simple_group_manager() : gcount_() {}

  inline size_t ngroups() const { return groups_.size(); }

  inline std::vector<size_t>
  groups() const
  {
    std::vector<size_t> ret;
    ret.reserve(ngroups());
    for (auto &g : groups_)
      ret.push_back(g.first);
    return ret;
  }

  inline std::pair<size_t, T&>
  create_group()
  {
    const size_t gid = gcount_++;
    auto &g = groups_[gid];
    return std::pair<size_t, T&>(gid, g);
  }

  inline void
  delete_group(size_t gid)
  {
    auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    groups_.erase(it);
  }

  inline const T &
  group(size_t gid) const
  {
    const auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    return it->second;
  }

  inline T &
  group(size_t gid)
  {
    auto it = groups_.find(gid);
    MICROSCOPES_DCHECK(it != groups_.end(), "invalid gid");
    return it->second;
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

private:
  size_t gcount_;
  std::map<size_t, T> groups_;
};

} // namespace common
} // namespace microscopes
