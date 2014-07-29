#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

namespace microscopes {
namespace common {

typedef size_t ident_t;
typedef void * opaque_t;
typedef std::string hyperparam_bag_t;
typedef std::string suffstats_bag_t;
typedef std::string serialized_t;

// R^{d} => R, for arbitrary finite d
typedef std::function<float(const std::vector<float> &)> scalar_fn; // for cython

} // namespace common
} // namespace microscopes
