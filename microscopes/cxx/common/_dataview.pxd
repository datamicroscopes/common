from libcpp cimport bool as cbool
from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

from microscopes.cxx.common._dataview_h cimport \
    dataview, row_major_dataview, row_accessor, row_mutator
from microscopes.cxx.common._rng cimport rng
cimport microscopes.cxx.common._type_info_h as ti
from microscopes.cxx.common._type_helper_h cimport runtime_type, RuntimeTypeStr

cimport numpy as np

cdef class abstract_dataview:
    cdef dataview *_thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef int _n
    cdef np.ndarray _data
    cdef np.ndarray _mask

cdef vector[runtime_type] get_c_types(dtype)
cdef np.dtype get_np_type(const runtime_type &)
