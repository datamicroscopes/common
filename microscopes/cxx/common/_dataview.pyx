import numpy as np

cdef class abstract_dataview:
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self._thisptr

cdef class numpy_dataview(abstract_dataview):
    def __cinit__(self, np.ndarray npd):
        self._npd = np.ascontiguousarray(npd) # bump refcount
        n = npd.shape[0]
        cdef vector[ti.runtime_type_info] ctypes
        ctypes = get_c_types(npd)
        cdef np.ndarray npd_mask
        if hasattr(npd, 'mask'):
            npd_mask = np.ascontiguousarray(npd.mask)
            self._thisptr = new row_major_dataview( <uint8_t *> npd.data, <cbool *> npd_mask.data, n, ctypes)
        else:
            self._thisptr = new row_major_dataview( <uint8_t *> npd.data, NULL, n, ctypes)
    def permute(self, rng rng):
        (<row_major_dataview *>self._thisptr)[0].permute(rng._thisptr[0])
        return self
    def reset_permutation(self):
        (<row_major_dataview *>self._thisptr)[0].reset_permutation()

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

cdef vector[ti.runtime_type_info] get_c_types(np.ndarray npd):
    cdef vector[ti.runtime_type_info] ctypes
    l = len(npd.dtype)
    for i in xrange(l):
        ctypes.push_back(get_c_type(npd.dtype[i]))
    return ctypes
