from microscopes.cxx.common._typedefs_h cimport scalar_1d_float_fn

cdef class scalar_function:
    cdef scalar_1d_float_fn _func
