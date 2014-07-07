#include <microscopes/models/niw.hpp>
#include <microscopes/io/schema.pb.h>
#include <distributions/random.hpp>
#include <distributions/special.hpp>

#include <stdexcept>
#include <sstream>
#include <cmath>
#include <limits>

#include <eigen3/Eigen/Cholesky>

using namespace std;
using namespace distributions;
using namespace microscopes::io;
using namespace microscopes::common;
using namespace microscopes::models;
using namespace Eigen;

typedef NormalInverseWishart_Shared shared_message_type;
typedef NormalInverseWishart_Group group_message_type;

void
niw_feature_group::add_value(const model &m, const row_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.curshape() == dim());

}

void
niw_feature_group::remove_value(const model &m, const row_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.curshape() == dim());
}

float
niw_feature_group::score_value(const model &m, const row_accessor &value, rng_t &rng) const
{
  MICROSCOPES_ASSERT(value.curshape() == dim());
  return 0.;
}

float
niw_feature_group::score_data(const model &m, rng_t &rng) const
{
  return 0.;
}

void
niw_feature_group::sample_value(const model &m, row_mutator &value, rng_t &rng) const
{
}

suffstats_bag_t
niw_feature_group::get_ss() const
{
  group_message_type m;
  m.set_count(count_);
  for (size_t i = 0; i < sum_x_.size(); i++)
    m.add_sum_x(sum_x_(i));
  for (size_t i = 0; i < dim(); i++)
    for (size_t j = 0; j < dim(); j++)
      m.add_sum_xxt(sum_xxT_(i, j));
  ostringstream out;
  m.SerializeToOstream(&out);
  return out.str();
}

void
niw_feature_group::set_ss(const suffstats_bag_t &ss)
{
  istringstream inp(ss);
  group_message_type m;
  m.ParseFromIstream(&inp);

  count_ = m.count();

  const size_t dim = m.sum_x_size();
  sum_x_.resize(dim, 1);
  for (size_t i = 0; i < dim; i++)
    sum_x_(i) = m.sum_x(i);

  MICROSCOPES_DCHECK(m.sum_xxt_size() == dim*dim, "sum_xxT must be D*D matrix");
  sum_xxT_.resize(dim, dim);

  for (size_t i = 0; i < dim; i++)
    for (size_t j = 0; j < dim; j++)
      sum_xxT_(i, j) = m.sum_xxt(i*dim + j);

  MICROSCOPES_DCHECK(
    sum_xxT_.isApprox(sum_xxT_.transpose()),
    "sum xx^T must be symmetric");

  LDLT<MatrixXf> ldlt;
  ldlt.compute(sum_xxT_);
  MICROSCOPES_DCHECK(
    ldlt.isPositive(),
    "sum xx^T must be positive (semi-) definite");
}

void *
niw_feature_group::get_ss_raw_ptr(const string &key)
{
  // XXX: support this safely
  throw runtime_error("unknown key: " + key);
}


shared_ptr<feature_group>
niw_model::create_feature_group(rng_t &rng) const
{
  return make_shared<niw_feature_group>(dim());
}

hyperparam_bag_t
niw_model::get_hp() const
{
  shared_message_type m;
  for (size_t i = 0; i < mu0_.size(); i++)
    m.add_mu0(mu0_(i));
  m.set_lambda(lambda_);
  for (size_t i = 0; i < psi_.rows(); i++)
    for (size_t j = 0; j < psi_.cols(); j++)
      m.add_psi(psi_(i, j));
  m.set_nu(nu_);
  ostringstream out;
  m.SerializeToOstream(&out);
  return out.str();
}

void
niw_model::set_hp(const hyperparam_bag_t &hp)
{
  istringstream inp(hp);
  shared_message_type m;
  m.ParseFromIstream(&inp);

  const size_t dim = m.mu0_size();
  mu0_.resize(dim, 1);
  for (size_t i = 0; i < dim; i++)
    mu0_(i) = m.mu0(i);

  lambda_ = m.lambda();
  MICROSCOPES_DCHECK(lambda_ > 0., "lambda must be positive");

  MICROSCOPES_DCHECK(m.psi_size() == dim*dim, "psi must be DxD matrix");

  psi_.resize(dim, dim);
  for (size_t i = 0; i < dim; i++)
    for (size_t j = 0; j < dim; j++)
      psi_(i, j) = m.psi(i*dim + j);

  MICROSCOPES_DCHECK(
    psi_.isApprox(psi_.transpose()),
    "psi must be symmetric");

  LDLT<MatrixXf> ldlt;
  ldlt.compute(psi_);
  MICROSCOPES_DCHECK(
    ldlt.isPositive(),
    "psi must be positive (semi-) definite");

  MICROSCOPES_DCHECK(m.nu() > float(dim) - 1., "DOF must be > D-1");
  nu_ = m.nu();
}

void
niw_model::set_hp(const model &m)
{
  *this = static_cast<const niw_model &>(m);
}

void *
niw_model::get_hp_raw_ptr(const string &key)
{
  // XXX: support this safely
  throw runtime_error("unknown key: " + key);
}

runtime_type
niw_model::get_runtime_type() const
{
  return runtime_type(TYPE_F32, dim());
}

string
niw_model::debug_str() const
{
  // XXX: implement me
  ostringstream oss;
  oss << "{NIW: XXX implement me}";
  return oss.str();
}
