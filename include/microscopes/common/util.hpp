#pragma once

#include <microscopes/common/random_fwd.hpp>

#include <iostream>
#include <vector>
#include <utility>
#include <random>

// pretty printer for std::pair<A, B>
template <typename A, typename B>
static inline std::ostream &
operator<<(std::ostream &o, const std::pair<A, B> &p)
{
  o << "[" << p.first << ", " << p.second << "]";
  return o;
}

// pretty printer for std::vector<T, Alloc>
template <typename T, typename Alloc>
static inline std::ostream &
operator<<(std::ostream &o, const std::vector<T, Alloc> &v)
{
  bool first = true;
  o << "[";
  for (auto &p : v) {
    if (!first)
      o << ", ";
    first = false;
    o << p;
  }
  o << "]";
  return o;
}

namespace microscopes {
namespace common {

struct util {

  static inline void
  inplace_range(std::vector<size_t> &a, size_t n)
  {
    a.clear();
    a.reserve(n);
    for (size_t i = 0; i < n; i++)
      a.push_back(i);
  }

  // generate a random permutation of the integers [0, ..., n-1]
  static inline void
  permute(std::vector<size_t> &pi, size_t n, rng_t &rng)
  {
    inplace_range(pi, n);
    for (size_t i = pi.size() - 1; i >= 1; i--) {
      std::uniform_int_distribution<> dist(0, i);
      const size_t j = dist(rng);
      std::swap(pi[j], pi[i]);
    }
  }

  static std::uniform_real_distribution<float> Unif01_;

  // copy from:
  // https://github.com/forcedotcom/distributions/blob/master/distributions/util.py

  static inline void
  scores_to_probs(std::vector<float> &scores)
  {
    const float m = *std::max_element(scores.begin(), scores.end());
    for (auto &s : scores)
      s -= m;
    for (auto &s : scores)
      s = expf(s);
    const float acc = std::accumulate(scores.begin(), scores.end(), 0.);
    for (auto &s : scores)
      s /= acc;
  }

  static inline size_t
  sample_discrete_log(std::vector<float> &scores, rng_t &rng)
  {
    scores_to_probs(scores);
    return sample_discrete(scores, rng);
  }

  static inline size_t
  sample_discrete(const std::vector<float> &probs, rng_t &rng)
  {
    // assumes probs add up to 1
    float dart = Unif01_(rng);
    for (size_t i = 0; i < probs.size(); i++) {
      dart -= probs[i];
      if (dart <= 0.)
        return i;
    }
    return probs.size() - 1;
  }

  static inline std::string
  hexify(const uint8_t *buf, size_t n)
  {
    const char *table = "0123456789abcdef";
    std::string res(n * 2, '0');
    for (size_t i = 0; i < n; i++) {
      res[i*2]   = table[buf[i] / 16];
      res[i*2+1] = table[buf[i] % 16];
    }
    return res;
  }

};

} // namespace common
} // namespace microscopes
