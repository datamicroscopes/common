
import numpy as np
import numpy.ma as ma

cdef class abstract_dataview:
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self._thisptr

cdef class numpy_dataview(abstract_dataview):
    def __cinit__(self, npd):
        pass
