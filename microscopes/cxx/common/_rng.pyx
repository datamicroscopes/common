import time

def _seed():
    import random
    return random.randint(0, 0x7FFFFFFF)

cdef class rng:
    def __cinit__(self, seed=_seed()):
        self._thisptr = new rng_t(seed)
    def __dealloc__(self):
        del self._thisptr
