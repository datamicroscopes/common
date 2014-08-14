from libcpp cimport bool as cbool
from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.common._dataview cimport get_c_types, get_np_type
from microscopes.common.recarray._dataview_h cimport (
    dataview,
    row_major_dataview,
    row_accessor,
    row_mutator,
)
from microscopes.common._rng cimport rng
cimport microscopes.common._type_info_h as ti
from microscopes.common._runtime_type_h cimport runtime_type

cimport numpy as np

cdef class abstract_dataview:
    cdef shared_ptr[dataview] _thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef int _n
    cdef readonly np.ndarray _data
    cdef readonly np.ndarray _mask
