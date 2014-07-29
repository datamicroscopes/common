from microscopes.cxx.common._typedefs_h cimport scalar_fn

cdef extern from "microscopes/common/scalar_functions.hpp" namespace "microscopes::common":
    scalar_fn log_exponential(float)
    scalar_fn log_normal(float, float)
    scalar_fn log_noninformative_beta_prior()

