import time
cdef class rng:
    def __cinit__(self, seed=int(time.time())):
        self._thisptr = new rng_t(seed)
    def __dealloc__(self):
        del self._thisptr
