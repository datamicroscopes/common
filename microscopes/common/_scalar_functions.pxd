from libcpp.vector cimport vector
from microscopes.common._scalar_functions_h cimport scalar_fn

cdef class scalar_function:
    cdef scalar_fn _func
