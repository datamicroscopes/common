from libcpp.utility cimport pair
from microscopes._eigen_h cimport VectorXf, MatrixXf
from microscopes.cxx.common._random_fwd_h cimport rng_t

cdef extern from "microscopes/common/random.hpp" namespace "microscopes::common::random":

    VectorXf sample_multivariate_normal(const VectorXf &, const MatrixXf &, rng_t &) except +
    MatrixXf sample_wishart(float, const MatrixXf &, rng_t &) except +
    MatrixXf sample_inverse_wishart(float, const MatrixXf &, rng_t &) except +
    pair[VectorXf, MatrixXf] sample_normal_inverse_wishart(const VectorXf &, float, const MatrixXf &, float, rng_t &) except +

