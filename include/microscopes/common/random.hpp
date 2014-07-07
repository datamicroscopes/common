#pragma once

#include <microscopes/common/random_fwd.hpp>
#include <microscopes/common/util.hpp>
#include <microscopes/common/assert.hpp>

#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Cholesky>

#include <cmath>
#include <random>
#include <utility>

namespace microscopes {
namespace common {

struct random {

  /**
   * Assumes sigma is positive definite
   */
  static inline Eigen::VectorXf
  sample_multivariate_normal(
      const Eigen::VectorXf &mu,
      const Eigen::MatrixXf &sigma,
      rng_t &rng)
  {
    MICROSCOPES_ASSERT(sigma.rows() == sigma.cols());
    MICROSCOPES_ASSERT(mu.size() == sigma.rows());
    MICROSCOPES_ASSERT(util::is_symmetric_positive_definite(sigma));

    Eigen::LLT<Eigen::MatrixXf> llt(sigma);
    MICROSCOPES_ASSERT(llt.info() == Eigen::Success);

    Eigen::VectorXf z(mu.size());
    std::normal_distribution<float> norm;
    for (unsigned i = 0; i < mu.size(); i++)
      z(i) = norm(rng);

    return mu + llt.matrixL() * z;
  }

  // Taken from:
  // http://www.mit.edu/~mattjj/released-code/hsmm/stats_util.py
  static inline Eigen::MatrixXf
  sample_wishart(float nu, const Eigen::MatrixXf &scale, rng_t &rng)
  {
    MICROSCOPES_ASSERT(scale.rows() == scale.cols());
    MICROSCOPES_ASSERT(util::is_symmetric_positive_definite(scale));

    Eigen::LLT<Eigen::MatrixXf> llt(scale);
    MICROSCOPES_ASSERT(llt.info() == Eigen::Success);

    Eigen::MatrixXf A(scale.rows(), scale.rows());
    for (unsigned i = 0; i < scale.rows(); i++)
      A(i, i) = sqrt(std::chi_squared_distribution<float>(nu - float(i))(rng));

    std::normal_distribution<float> norm;
    for (unsigned i = 1; i < scale.rows(); i++)
      for (unsigned j = 0; j < i; j++)
        A(i, j) = norm(rng);

    const Eigen::MatrixXf &X = llt.matrixL() * A;
    return X * X.transpose();
  }

  static inline Eigen::MatrixXf
  sample_inverse_wishart(float nu, const Eigen::MatrixXf &psi, rng_t &rng)
  {
    // XXX: horrible
    const Eigen::MatrixXf &psi_inv = psi.inverse();
    const Eigen::MatrixXf &sigma_inv = sample_wishart(nu, psi_inv, rng);
    return sigma_inv.inverse();
  }

  static inline std::pair<Eigen::VectorXf, Eigen::MatrixXf>
  sample_normal_inverse_wishart(const Eigen::VectorXf &mu0, float lambda, const Eigen::MatrixXf &psi, float nu, rng_t &rng)
  {
    const Eigen::MatrixXf &cov = 1./lambda * sample_inverse_wishart(nu, psi, rng);
    const Eigen::VectorXf &mu = sample_multivariate_normal(mu0, cov, rng);
    return std::make_pair(mu, cov);
  }
};

} // namespace common
} // namespace microscopes
