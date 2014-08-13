# cython: embedsignature=True


import numpy as np
from microscopes.common import validator

cdef class abstract_dataview:
    pass

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
        self._data = np.ascontiguousarray(npd.data)
        if hasattr(npd, 'mask'):
            self._mask = np.ascontiguousarray(npd.mask)
        else:
            self._mask = None
        self._thisptr.reset(new row_major_dense_dataview(
            <uint8_t *> self._data.data,
            <cbool *> self._mask.data if self._mask is not None else NULL,
            cshape,
            ctype))

    def shape(self):
        return self._shape

cdef class sparse_2d_dataview(abstract_dataview):
    def __cinit__(self, rep):
        """
        only takes scipy.sparse arrays
        """
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
        self._csr_data, self._csr_indices, self._csr_indptr = \
            csr_rep.data, csr_rep.indices, csr_rep.indptr
        self._csc_data, self._csc_indices, self._csc_indptr = \
            csc_rep.data, csc_rep.indices, csc_rep.indptr

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
