# cython: embedsignature=True


import numpy as np
from microscopes.common import validator


cdef class abstract_dataview:

    def digest(self, h):

        # two different object types should not collide
        typ = type(self)
        fqn = typ.__module__ + '.' + typ.__name__
        h.update(fqn)

        # implementations now fill out the details
        self._digest(h)

        return h


cdef class numpy_dataview(abstract_dataview):
    def __cinit__(self, data):
        validator.validate_not_none(data, "data")
        self._data = list(data)
        dtype = self._data[0].dtype
        for i, d in enumerate(self._data):
            if len(d.shape) != 1:
                raise ValueError("1-d arrays only")
            self._data[i] = np.ascontiguousarray(d)
            if dtype != d.dtype:
                # XXX: should co-erce to common type
                raise ValueError("inconsistent types")

        cdef vector[const uint8_t *] pxs
        cdef vector[unsigned] ns
        for d in self._data:
            pxs.push_back(<const uint8_t *> (<np.ndarray>d).data)
            ns.push_back(len(d))

        cdef runtime_type ctype = get_c_type(dtype)

        self._thisptr.reset(new row_major_dataview(pxs, ns, ctype))

    def size(self):
        return len(self._data)

    def __len__(self):
        return self.size()

    def _digest(self, h):
        # use the str repr for dtype
        h.update(str(self._data[0].dtype))

        # XXX(stephentu): this is probably really slow
        for d in self._data:
            h.update(d.view(np.uint8))
