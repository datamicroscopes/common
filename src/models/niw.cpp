#include <microscopes/models/niw.hpp>
#include <microscopes/io/schema.pb.h>
#include <microscopes/common/special.hpp>
#include <microscopes/common/random.hpp>
#include <microscopes/common/util.hpp>

#include <distributions/random.hpp>
#include <distributions/special.hpp>

#include <stdexcept>
#include <sstream>
#include <cmath>
#include <limits>

#include <eigen3/Eigen/Dense>

using namespace std;
using namespace distributions;
using namespace microscopes::io;
using namespace microscopes::common;
using namespace microscopes::models;
using namespace Eigen;

typedef NormalInverseWishart_Shared shared_message_type;
typedef NormalInverseWishart_Group group_message_type;

static inline void
extractVec(VectorXf &v, const value_accessor &value)
{
  for (size_t i = 0; i < v.size(); i++)
    v(i) = value.get<float>(i);
}

static inline float
score_student_t(
    const VectorXf &v,
    float nu,
    const VectorXf &mu,
    const MatrixXf &sigma)
{
  const unsigned d = v.size();
  const float term1 = fast_lgamma(nu/2. + float(d)/2.) - fast_lgamma(nu/2.);

  // XXX: this is horribly inefficient, I know.
  // deal with it for now

  // XXX: use Cholesky decomposition to make this faster
  const MatrixXf &sigma_inv = sigma.inverse();
  const float sigma_det = sigma.determinant();

  const float term2 = -0.5*fast_log(sigma_det) - float(d)/2.*(
    fast_log(nu) + 1.1447298858494002 /* log(pi) */);

  const VectorXf &diff = v - mu;

  const float term3 = -0.5*(nu+float(d))*fast_log(1.+1./nu*(  diff.dot(sigma_inv*diff)   ));

  return term1 + term2 + term3;
}

void
niw_group::postParams(const niw_hypers &m, suffstats_t &ss) const
{
  const float n = count_;
  VectorXf xbar;
  if (count_)
    xbar = sum_x_ / n;
  else
    xbar = VectorXf::Zero(dim());
  ss.mu0_ = m.lambda_/(m.lambda_+n)*m.mu0_ + n/(m.lambda_+n)*xbar;
  ss.lambda_ = m.lambda_ + n;
  ss.nu_ = m.nu_ + n;
  const VectorXf &diff = xbar - m.mu0_;
  const MatrixXf &C_n =
    sum_xxT_ - sum_x_*xbar.transpose() - xbar*sum_x_.transpose() + n*xbar*xbar.transpose();
  ss.psi_ = m.psi_ + C_n + m.lambda_*n/(m.lambda_+n)*diff*diff.transpose();

  //cout << "postParams(): " << endl
  //     << "  mu0_: " << ss.mu0_ << endl
  //     << "  lambda_: " << ss.lambda_ << endl
  //     << "  psi_: " << ss.psi_ << endl
  //     << "  nu_: " << ss.nu_ << endl;
}

void
niw_group::add_value(const hypers &m, const value_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.shape() == dim());
  count_++;
  VectorXf v(dim());
  extractVec(v, value);
  sum_x_ += v;
  sum_xxT_ += v * v.transpose();
}

void
niw_group::remove_value(const hypers &m, const value_accessor &value, rng_t &rng)
{
  MICROSCOPES_ASSERT(value.shape() == dim());
  MICROSCOPES_ASSERT(count_ > 0);
  count_--;
  VectorXf v(dim());
  extractVec(v, value);
  sum_x_ -= v;
  sum_xxT_ -= v * v.transpose();
}

float
niw_group::score_value(const hypers &m, const value_accessor &value, rng_t &rng) const
{
  MICROSCOPES_ASSERT(value.shape() == dim());
  VectorXf v(dim());
  extractVec(v, value);
  suffstats_t ss;
  postParams(static_cast<const niw_hypers &>(m), ss);
  const float dof = ss.nu_ - float(dim()) + 1.;
  const MatrixXf sigma = ss.psi_ * (ss.lambda_+1.)/(ss.lambda_*dof);
  //cout << "niw_group::score_value" << endl
  //     << "  dof: " << dof << endl
  //     << "  sigma: " << sigma << endl;
  return score_student_t(v, dof, ss.mu0_, sigma);
}

float
niw_group::score_data(const hypers &m, rng_t &rng) const
{
  const niw_hypers &m1 = static_cast<const niw_hypers &>(m);
  suffstats_t ss;
  postParams(m1, ss);

  //const auto T1 = special::lmultigamma(dim(), ss.nu_*0.5);
  //const auto T2 = + m1.nu_*0.5*fast_log(m1.psi_.determinant());
  //const auto T3 = - float(count_*dim())*0.5*1.1447298858494002 /* log(pi) */;
  //const auto T4 = - special::lmultigamma(dim(), m1.nu_*0.5);
  //const auto T5 = - ss.nu_*0.5*fast_log(ss.psi_.determinant());
  //const auto T6 = + float(dim())*0.5*fast_log(m1.lambda_/ss.lambda_);

  //cout << "cxx T1 " << T1 << " D " << dim() << " nu_n/2 " << ss.nu_*0.5 << endl;
  //cout << "cxx T2 " << T2 << endl;
  //cout << "cxx T3 " << T3 << endl;
  //cout << "cxx T4 " << T4 << endl;
  //cout << "cxx T5 " << T5 << endl;
  //cout << "cxx T6 " << T6 << endl;

  return special::lmultigamma(dim(), ss.nu_*0.5)
    + m1.nu_*0.5*fast_log(m1.psi_.determinant())
    - float(count_*dim())*0.5*1.1447298858494002 /* log(pi) */
    - special::lmultigamma(dim(), m1.nu_*0.5)
    - ss.nu_*0.5*fast_log(ss.psi_.determinant())
    + float(dim())*0.5*fast_log(m1.lambda_/ss.lambda_);
}

void
niw_group::sample_value(const hypers &m, value_mutator &value, rng_t &rng) const
{
  suffstats_t ss;
  postParams(static_cast<const niw_hypers &>(m), ss);
  const auto p = random::sample_normal_inverse_wishart(ss.mu0_, ss.lambda_, ss.psi_, ss.nu_, rng);
  const VectorXf &x = random::sample_multivariate_normal(p.first, p.second, rng);
  MICROSCOPES_ASSERT(x.size() == value.shape());
  for (unsigned i = 0; i < x.size(); i++)
    value.set<float>(x(i), i);
}

suffstats_bag_t
niw_group::get_ss() const
{
  group_message_type m;
  m.set_count(count_);
  for (size_t i = 0; i < sum_x_.size(); i++)
    m.add_sum_x(sum_x_(i));
  for (size_t i = 0; i < dim(); i++)
    for (size_t j = 0; j < dim(); j++)
      m.add_sum_xxt(sum_xxT_(i, j));
  return util::protobuf_to_string(m);
}

void
niw_group::set_ss(const suffstats_bag_t &ss)
{
  group_message_type m;
  util::protobuf_from_string(m, ss);

  count_ = m.count();

  const size_t dim = m.sum_x_size();
  MICROSCOPES_DCHECK(sum_x_.size() == dim, "size mismatch");
  for (size_t i = 0; i < dim; i++)
    sum_x_(i) = m.sum_x(i);

  MICROSCOPES_DCHECK(size_t(m.sum_xxt_size()) == dim*dim, "sum_xxT must be D*D matrix");

  for (size_t i = 0; i < dim; i++)
    for (size_t j = 0; j < dim; j++)
      sum_xxT_(i, j) = m.sum_xxt(i*dim + j);

  MICROSCOPES_DCHECK(util::is_symmetric_positive_definite(sum_xxT_), "sum xx^T must be SPD");
}

void
niw_group::set_ss(const group &m)
{
  *this = static_cast<const niw_group &>(m);
}

value_mutator
niw_group::get_ss_mutator(const string &key)
{
  // XXX: support this safely
  throw runtime_error("unknown key: " + key);
}

string
niw_group::debug_str() const
{
  // XXX: implement me
  ostringstream oss;
  oss << "{NIW: XXX implement me}";
  return oss.str();
}

shared_ptr<group>
niw_hypers::create_group(rng_t &rng) const
{
  return make_shared<niw_group>(dim());
}

hyperparam_bag_t
niw_hypers::get_hp() const
{
  shared_message_type m;
  for (size_t i = 0; i < mu0_.size(); i++)
    m.add_mu0(mu0_(i));
  m.set_lambda(lambda_);
  for (size_t i = 0; i < psi_.rows(); i++)
    for (size_t j = 0; j < psi_.cols(); j++)
      m.add_psi(psi_(i, j));
  m.set_nu(nu_);
  return util::protobuf_to_string(m);
}

void
niw_hypers::set_hp(const hyperparam_bag_t &hp)
{
  shared_message_type m;
  util::protobuf_from_string(m, hp);

  const size_t dim = m.mu0_size();
  MICROSCOPES_DCHECK(mu0_.rows() == dim, "size mismatch");
  for (size_t i = 0; i < dim; i++)
    mu0_(i) = m.mu0(i);

  lambda_ = m.lambda();
  MICROSCOPES_DCHECK(lambda_ > 0., "lambda must be positive");

  MICROSCOPES_DCHECK(size_t(m.psi_size()) == dim*dim, "psi must be DxD matrix");

  for (size_t i = 0; i < dim; i++)
    for (size_t j = 0; j < dim; j++)
      psi_(i, j) = m.psi(i*dim + j);

  MICROSCOPES_DCHECK(util::is_symmetric_positive_definite(psi_), "psi must be SPD");
  MICROSCOPES_DCHECK(m.nu() > float(dim) - 1., "DOF must be > D-1");
  nu_ = m.nu();
}

void
niw_hypers::set_hp(const hypers &m)
{
  *this = static_cast<const niw_hypers &>(m);
}

value_mutator
niw_hypers::get_hp_mutator(const string &key)
{
  // XXX: support this safely
  throw runtime_error("unknown key: " + key);
}

string
niw_hypers::debug_str() const
{
  // XXX: implement me
  ostringstream oss;
  oss << "{NIW: XXX implement me}";
  return oss.str();
}

runtime_type
niw_model::get_runtime_type() const
{
  return runtime_type(TYPE_F32, dim());
}
