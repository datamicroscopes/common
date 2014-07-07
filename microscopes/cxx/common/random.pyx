from libcpp.utility cimport pair
from microscopes.cxx.common._random_h cimport \
        sample_multivariate_normal as c_sample_multivariate_normal, \
        sample_wishart as c_sample_wishart, \
        sample_inverse_wishart as c_sample_inverse_wishart, \
        sample_normal_inverse_wishart as c_sample_normal_inverse_wishart
from microscopes.cxx.common._rng cimport rng
from microscopes.cxx.common._util_h cimport \
        set as c_eigen_matf_set, \
        get as c_eigen_matf_get
from microscopes._eigen_h cimport VectorXf, MatrixXf

cimport numpy as np
import numpy as np

cdef VectorXf to_eigen_vecf(x):
    assert len(x.shape) == 1
    cdef VectorXf v = VectorXf(x.shape[0])
    for i, e in enumerate(x):
        v[i] = float(e)
    return v

cdef MatrixXf to_eigen_matf(x):
    assert len(x.shape) == 2
    cdef MatrixXf m = MatrixXf(x.shape[0], x.shape[1])
    for i, a in enumerate(x):
        for j, b in enumerate(a):
            c_eigen_matf_set(m, i, j, b) 
    return m

cdef np.ndarray to_np_1darray(const VectorXf &x):
    cdef np.ndarray v = np.zeros(x.size())
    for i in xrange(x.size()):
        v[i] = x[i]
    return v

cdef np.ndarray to_np_2darray(const MatrixXf &x):
    cdef np.ndarray m = np.zeros((x.rows(), x.cols()))
    for i in xrange(x.rows()):
        for j in xrange(x.cols()):
            m[i, j] = c_eigen_matf_get(x, i, j)
    return m

def sample_multivariate_normal(np.ndarray mu, np.ndarray cov, rng r):
    cdef VectorXf sample = c_sample_multivariate_normal(
        to_eigen_vecf(mu),
        to_eigen_matf(cov),
        r._thisptr[0])
    return to_np_1darray(sample)

def sample_wishart(float nu, np.ndarray scale, rng r):
    cdef MatrixXf sample = c_sample_wishart(
        nu,
        to_eigen_matf(scale),
        r._thisptr[0])
    return to_np_2darray(sample)

def sample_inverse_wishart(float nu, np.ndarray scale, rng r):
    cdef MatrixXf sample = c_sample_inverse_wishart(
        nu,
        to_eigen_matf(scale),
        r._thisptr[0])
    return to_np_2darray(sample)

def sample_normal_inverse_wishart(np.ndarray mu0, float lam, np.ndarray psi, float nu, rng r):
    cdef pair[VectorXf, MatrixXf] sample = c_sample_normal_inverse_wishart(
        to_eigen_vecf(mu0),
        lam,
        to_eigen_matf(psi),
        nu,
        r._thisptr[0])
    return to_np_1darray(sample.first), to_np_2darray(sample.second)
