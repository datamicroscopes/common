import numpy as np
import numpy.ma as ma

cdef class abstract_dataview:
    def __iter__(self):
        self._thisptr.get().reset()
        return self
    def next(self):
        if self._thisptr.get().end():
            raise StopIteration
        cdef row_accessor acc = self._thisptr.get().get()
        cdef const vector[runtime_type] *types = &self._thisptr.get().types()
        dtypes = []
        for i in xrange(types.size()):
            dtypes.append(('', get_np_type(types[0][i])))
        cdef np.ndarray array = np.zeros(1, dtype=dtypes)
        self._thisptr.get().next()
        cdef row_mutator mut = row_mutator(<uint8_t *> array.data, types)
        masks = []
        has_any_masks = [False]
        def mark(b):
            if b:
                has_any_masks[0] = True
            return b
        for i in xrange(types.size()):
            mut.set(acc)
            m = [mark(acc.ismasked(j)) for j in xrange(acc.curshape())]
            if not acc.curtype().vec():
                assert len(m) == 1
                m = m[0]
            masks.append(m)
            mut.bump()
            acc.bump()
        if not has_any_masks[0]:
            return array[0]
        masks = tuple(masks) # this seems to matter
        return ma.array(array, mask=[masks])[0]

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
            self._thisptr.reset(new row_major_dataview(
                <uint8_t *> self._data.data,
                <cbool *> self._mask.data,
                n,
                ctypes))
        else:
            self._thisptr.reset(new row_major_dataview(
                <uint8_t *> self._data.data,
                NULL,
                n,
                ctypes))

    def view(self, shuffle, rng r):
        if not shuffle:
            (<row_major_dataview *>self._thisptr.get()).reset_permutation()
        else:
            (<row_major_dataview *>self._thisptr.get()).permute(r._thisptr[0])
        return self

    def size(self):
        return self._n
