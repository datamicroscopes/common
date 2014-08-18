# cython: embedsignature=True


import numpy as np
import numpy.ma as ma
import hashlib
from microscopes.common import validator


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
        masks = tuple(masks)  # this seems to matter
        return ma.array(array, mask=[masks])[0]

    def digest(self):
        h = hashlib.sha1()

        # two different object types should not collide
        typ = type(self)
        fqn = typ.__module__ + '.' + typ.__name__
        h.update(fqn)

        # implementations now fill out the details
        self._digest(h)

        return h.hexdigest()


cdef class numpy_dataview(abstract_dataview):
    def __cinit__(self, npd):
        validator.validate_not_none(npd, "npd")
        if len(npd.shape) != 1:
            raise ValueError("1D (structural) arrays only")
        self._n = npd.shape[0]
        dtype = npd.dtype
        if len(dtype) == 0:
            # XXX(stephentu): is there a better (less heuristic) way of
            # checking for this
            raise ValueError("structural arrays only")

        if hasattr(npd, 'mask'):
            self._data = np.ascontiguousarray(npd.data)
            self._mask = np.ascontiguousarray(npd.mask)
        else:
            self._data = np.ascontiguousarray(npd)
            self._mask = None

        cdef vector[runtime_type] ctypes = get_c_types(dtype)

        if self._mask is not None:
            self._thisptr.reset(new row_major_dataview(
                <uint8_t *> self._data.data,
                <cbool *> self._mask.data,
                self._n,
                ctypes))
        else:
            self._thisptr.reset(new row_major_dataview(
                <uint8_t *> self._data.data,
                NULL,
                self._n,
                ctypes))

    def size(self):
        return self._n

    def __len__(self):
        return self.size()

    def _digest(self, h):
        if self._mask is not None:
            # XXX(stephentu): implement me
            raise NotImplementedError(
                "masked arrays digest not implemented")

        # use the str repr for dtype
        h.update(str(self._data.dtype))

        # now take a memory view of the data (since we already know it is
        # contiguous in row-major order, we don't have to worry about column
        # major arrays)
        h.update(self._data.view(np.uint8))
