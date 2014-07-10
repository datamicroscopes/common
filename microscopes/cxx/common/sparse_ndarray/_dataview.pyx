import numpy as np
#import numpy.ma as ma

cdef class abstract_dataview:
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self._thisptr

cdef class numpy_dataview(abstract_dataview):
    def __cinit__(self, npd):
        if len(npd.shape) <= 1:
            raise ValueError("dim must be >= 2")
        if len([x for x in npd.shape if not x]):
            raise ValueError("empty dims not allowed")
        cdef vector[size_t] cshape
        for d in npd.shape:
            cshape.push_back(d)
        cdef runtime_type ctype = get_c_type(npd.dtype)
        self._data = np.ascontiguousarray(npd.data)
        if hasattr(npd, 'mask'):
            self._mask = np.ascontiguousarray(npd.mask)
        else:
            self._mask = None
        self._thisptr = new row_major_dense_dataview(
            <uint8_t *> self._data.data,
            <cbool *> self._mask.data if self._mask is not None else NULL,
            cshape,
            ctype)
