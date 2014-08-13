# cython: embedsignature=True


from __future__ import absolute_import

import time
import random


def _seed():
    return random.randint(0, 0x7FFFFFFF)


cdef class rng:
    def __cinit__(self, seed=_seed()):
        self._thisptr = new rng_t(seed)

    def __dealloc__(self):
        del self._thisptr
