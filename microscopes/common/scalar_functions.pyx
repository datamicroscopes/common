# cython: embedsignature=True


from microscopes.common._scalar_functions cimport scalar_function
from microscopes.common._scalar_functions_h cimport (
    log_exponential as c_log_exponential,
    log_normal as c_log_normal,
    log_noninformative_beta_prior as c_log_noninformative_beta_prior,
)


cdef class log_exponential(scalar_function):
    cdef readonly float _lam

    def __cinit__(self, float lam):
        self._lam = lam
        self._func = c_log_exponential(lam)

    def __reduce__(self):
        return (_reconstruct_log_exponential, (self._lam,))


cdef class log_normal(scalar_function):
    cdef readonly float _mu
    cdef readonly float _sigma2

    def __cinit__(self, float mu, float sigma2):
        self._func = c_log_normal(mu, sigma2)

    def __reduce__(self):
        return (_reconstruct_log_normal, (self._mu, self._sigma2,))


cdef class _log_noninformative_beta_prior(scalar_function):
    def __cinit__(self):
        self._func = c_log_noninformative_beta_prior()

    def __reduce__(self):
        return (_reconstruct_log_noninformative_beta_prior, ())


def _reconstruct_log_exponential(lam):
    return log_exponential(lam)


def _reconstruct_log_normal(mu, sigma2):
    return log_normal(mu, sigma2)


def _reconstruct_log_noninformative_beta_prior():
    return log_noninformative_beta_prior


log_noninformative_beta_prior = _log_noninformative_beta_prior()
