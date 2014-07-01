from microscopes.cxx.common._typedefs_h cimport scalar_1d_float_fn

cdef extern from "microscopes/common/scalar_functions.hpp" namespace "microscopes::common":
    scalar_1d_float_fn log_exponential(float)
    scalar_1d_float_fn log_normal(float, float)
