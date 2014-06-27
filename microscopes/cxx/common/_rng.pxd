from microscopes.cxx.common._random_fwd_h cimport rng_t 
cdef class rng:
    cdef rng_t *_thisptr
