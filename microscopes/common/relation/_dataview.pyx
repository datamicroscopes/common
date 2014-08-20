# cython: embedsignature=True


import numpy as np
import numpy.ma as ma
from scipy.sparse import (
    csr_matrix,
    csc_matrix,
)
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

    def __cinit__(self, npd):
        validator.validate_not_none(npd, "npd")
        if len(npd.shape) <= 1:
            raise ValueError("dim must be >= 2")
        if len([x for x in npd.shape if not x]):
            raise ValueError("empty dims not allowed")
        self._shape = npd.shape
        cdef vector[size_t] cshape
        for d in npd.shape:
            cshape.push_back(d)
        cdef runtime_type ctype = get_c_type(npd.dtype)
        if hasattr(npd, 'mask'):
            self._data = np.ascontiguousarray(npd.data)
            self._mask = np.ascontiguousarray(npd.mask)
        else:
            self._data = np.ascontiguousarray(npd)
            self._mask = None
        self._thisptr.reset(new row_major_dense_dataview(
            <uint8_t *> self._data.data,
            <cbool *> self._mask.data if self._mask is not None else NULL,
            cshape,
            ctype))

    def shape(self):
        return self._shape

    def toarray(self):
        if self._mask is None:
            return self._data
        else:
            return ma.array(self._data, mask=self._mask)

    def _digest(self, h):
        # use the str repr for dtype
        h.update(str(self._data.dtype))

        if self._mask is None:
            # fast implementation
            h.update(self._data.view(np.uint8))
        else:
            h.update(self._mask.view(np.uint8))

            # slow implementation-- we have to ensure the masked values have
            # the same value
            data, mask = np.ravel(self._data), np.ravel(self._mask)
            data[mask] = 0  # XXX(stephentu): what if zero is not valid

            h.update(data.view(np.uint8))


cdef class sparse_2d_dataview(abstract_dataview):

    def __cinit__(self, rep):
        self._rows, self._cols = rep.shape
        validator.validate_positive(self._rows)
        validator.validate_positive(self._cols)

        csr_rep = rep.tocsr()
        csc_rep = rep.tocsc()

        if csr_rep.data.dtype != csc_rep.data.dtype:
            raise RuntimeError("dtypes don't match")
        cdef runtime_type ctype = get_c_type(csr_rep.data.dtype)

        def validate_sparse_rep(rep):
            if rep.indices.dtype != np.dtype('int32'):
                raise RuntimeError("expected i32 indices")
            if rep.indptr.dtype != np.dtype('int32'):
                raise RuntimeError("expected i32 indptr")

        validate_sparse_rep(csr_rep)
        validate_sparse_rep(csc_rep)

        # keep the refcounts live
        self._csr_data, self._csr_indices, self._csr_indptr = (
            csr_rep.data, csr_rep.indices, csr_rep.indptr
        )
        self._csc_data, self._csc_indices, self._csc_indptr = (
            csc_rep.data, csc_rep.indices, csc_rep.indptr
        )

        self._thisptr.reset(
            new compressed_2darray(
                <const uint8_t *> self._csr_data.data,
                <const uint32_t *> self._csr_indices.data,
                <const uint32_t *> self._csr_indptr.data,
                <const uint8_t *> self._csc_data.data,
                <const uint32_t *> self._csc_indices.data,
                <const uint32_t *> self._csc_indptr.data,
                self._rows,
                self._cols,
                ctype))

    def shape(self):
        return (self._rows, self._cols)

    def tocsr(self):
        return csr_matrix(
            (self._csr_data, self._csr_indices, self._csr_indptr),
            shape=self.shape())

    def tocsc(self):
        return csc_matrix(
            (self._csc_data, self._csc_indices, self._csc_indptr),
            shape=self.shape())

    def tocoo(self):
        return self.tocsr().tocoo()

    def _digest(self, h):

        # use the str repr for dtype
        h.update(str(self._csr_data.dtype))

        h.update(str(self.shape()))

        # digest the CSR representation
        h.update(self._csr_data.view(np.uint8))
        h.update(self._csr_indices.view(np.uint8))
        h.update(self._csr_indptr.view(np.uint8))
