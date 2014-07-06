import numpy as np

cdef class abstract_dataview:
    def __cinit__(self):
        pass
    def __dealloc__(self):
        del self._thisptr
    def __iter__(self):
        self._thisptr[0].reset()
        return self
    def next(self):
        if self._thisptr[0].end():
            raise StopIteration
        cdef row_accessor acc = self._thisptr[0].get()
        cdef const vector[runtime_type] *types = &self._thisptr[0].types()
        dtypes = []
        for i in xrange(types.size()):
            dtypes.append(('', get_np_type(types[0][i])))
        cdef np.ndarray array = np.zeros(1, dtype=dtypes)
        self._thisptr[0].next()
        cdef row_mutator mut = row_mutator(<uint8_t *> array.data, types)
        for i in xrange(types.size()):
            mut.set(acc)
            mut.bump()
            acc.bump()
        return array[0]

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

        cdef vector[runtime_type] ctypes
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

TYPES = (
    ('bool'   , ti.TYPE_B)   , 
    ('int8'   , ti.TYPE_I8)  , 
    ('uint8'  , ti.TYPE_U8)  , 
    ('int16'  , ti.TYPE_I16) , 
    ('uint16' , ti.TYPE_U16) , 
    ('int32'  , ti.TYPE_I32) , 
    ('uint32' , ti.TYPE_U32) , 
    ('int64'  , ti.TYPE_I64) , 
    ('uint64' , ti.TYPE_U64) , 
    ('f4'     , ti.TYPE_F32) , 
    ('f8'     , ti.TYPE_F64) , 
)

def get_c_type(tpe):
    for name, ctype in TYPES:
        if np.dtype(name) == tpe:
            return ctype
    raise ValueError("Unknown type: " + tpe)

cdef vector[runtime_type] get_c_types(dtype):
    cdef vector[runtime_type] ctypes
    ctypes.reserve(len(dtype))
    for i in xrange(len(dtype)):
        if dtype[i].subdtype is None:
            # scalar field
            ctypes.push_back(runtime_type(get_c_type(dtype[i])))
        else:
            # vector field
            subdtype, shape = dtype[i].subdtype
            if len(shape) != 1:
                raise ValueError("unsupported shape: " + shape)
            ctypes.push_back(runtime_type(get_c_type(subdtype), shape[0]))
    return ctypes

cdef np.dtype get_np_type(const runtime_type & tpe):
    for name, ctype in TYPES:
        if tpe.t() != ctype:
            continue
        if tpe.vec():
            name = '({},){}'.format(tpe.n(), name)
        return np.dtype(name)
    raise Exception("unknown type: " + RuntimeTypeStr(tpe))
