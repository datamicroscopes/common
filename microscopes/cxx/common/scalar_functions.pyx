from microscopes.cxx.common._scalar_functions cimport scalar_function
from microscopes.cxx.common._typedefs_h cimport scalar_1d_float_fn
from microscopes.cxx.common._scalar_functions_h cimport log_exponential as c_log_exponential

cdef class log_exponential(scalar_function):
    def __cinit__(self, float lam):
        self._func = c_log_exponential(lam) 
