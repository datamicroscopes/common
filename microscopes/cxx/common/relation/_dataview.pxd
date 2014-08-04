from libcpp.vector cimport vector
from libcpp cimport bool as cbool
from libc.stdint cimport uint8_t, uint32_t

from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx.common.relation._dataview_h cimport \
    dataview, row_major_dense_dataview, compressed_2darray
from microscopes.cxx.common._dataview cimport get_c_type
from microscopes.cxx.common._runtime_type_h cimport runtime_type

cimport numpy as np

cdef class abstract_dataview:
    cdef shared_ptr[dataview] _thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef np.ndarray _data
    cdef np.ndarray _mask
    cdef tuple _shape

cdef class sparse_2d_dataview(abstract_dataview):
    cdef np.ndarray _csr_data
    cdef np.ndarray _csr_indices
    cdef np.ndarray _csr_indptr
    cdef np.ndarray _csc_data
    cdef np.ndarray _csc_indices
    cdef np.ndarray _csc_indptr
    cdef int _rows
    cdef int _cols
