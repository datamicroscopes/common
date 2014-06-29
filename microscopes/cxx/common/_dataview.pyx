import numpy as np

cdef class abstract_dataview:
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self._thisptr

cdef class numpy_dataview(abstract_dataview):
    def __cinit__(self, npd):
        n = npd.shape[0]
        if len(npd.shape) != 1:
            raise ValueError("1D arrays only")
        self._n = n
        dtype = npd.dtype
        if len(dtype) == 0:
            raise ValueError("structural arrays only")

        self._data = np.ascontiguousarray(npd.data)
        if hasattr(npd, 'mask'):
            self._mask = np.ascontiguousarray(npd.mask)
        else:
            self._mask = None

        cdef vector[ti.runtime_type_info] ctypes
        ctypes = get_c_types(dtype)

        if self._mask is not None:
            self._thisptr = new row_major_dataview( 
                <uint8_t *> self._data.data, 
                <cbool *> self._mask.data, 
                n, 
                ctypes)
        else:
            self._thisptr = new row_major_dataview( 
                <uint8_t *> self._data.data, 
                NULL, 
                n, 
                ctypes)

    def view(self, shuffle, rng r):
        if not shuffle:
            (<row_major_dataview *>self._thisptr)[0].reset_permutation()
        else:
            (<row_major_dataview *>self._thisptr)[0].permute(r._thisptr[0])
        return self

    def size(self):
        return self._n

def get_c_type(tpe):
    if tpe in (bool, np.bool):
        return ti.TYPE_INFO_B
    if tpe == np.int8:
        return ti.TYPE_INFO_I8
    if tpe == np.int16:
        return ti.TYPE_INFO_I16
    if tpe == np.int32:
        return ti.TYPE_INFO_I32
    if tpe in (int, np.int, np.int64):
        return ti.TYPE_INFO_I64
    if tpe == np.float32:
        return ti.TYPE_INFO_F32
    if tpe in (float, np.float, np.float64):
        return ti.TYPE_INFO_F64
    raise Exception("Unknown type: " + tpe)

cdef vector[ti.runtime_type_info] get_c_types(dtype):
    cdef vector[ti.runtime_type_info] ctypes
    for i in xrange(len(dtype)):
        ctypes.push_back(get_c_type(dtype[i]))
    return ctypes
