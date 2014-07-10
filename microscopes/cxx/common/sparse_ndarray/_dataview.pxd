from libcpp.vector cimport vector
from libcpp cimport bool as cbool
from libc.stdint cimport uint8_t

from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx.common.sparse_ndarray._dataview_h cimport dataview, row_major_dense_dataview
from microscopes.cxx.common._dataview cimport get_c_type
from microscopes.cxx.common._runtime_type_h cimport runtime_type

cimport numpy as np

cdef class abstract_dataview:
    cdef shared_ptr[dataview] _thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef np.ndarray _data
    cdef np.ndarray _mask
