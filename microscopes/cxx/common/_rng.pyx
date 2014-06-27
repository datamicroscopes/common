cdef class rng:
    def __cinit__(self, seed=12345):
        self._thisptr = new rng_t(seed)
    def __dealloc__(self):
        del self._thisptr
