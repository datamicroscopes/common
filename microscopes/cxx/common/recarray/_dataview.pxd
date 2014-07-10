from libcpp cimport bool as cbool
from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx.common._dataview cimport get_c_types, get_np_type
from microscopes.cxx.common.recarray._dataview_h cimport \
    dataview, row_major_dataview, row_accessor, row_mutator
from microscopes.cxx.common._rng cimport rng
cimport microscopes.cxx.common._type_info_h as ti
from microscopes.cxx.common._runtime_type_h cimport runtime_type, RuntimeTypeStr

cimport numpy as np

cdef class abstract_dataview:
    cdef shared_ptr[dataview] _thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef int _n
    cdef np.ndarray _data
    cdef np.ndarray _mask
