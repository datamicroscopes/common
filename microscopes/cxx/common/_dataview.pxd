from libcpp cimport bool as cbool
from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

from microscopes.cxx.common._dataview_h cimport dataview, row_major_dataview
from microscopes.cxx.common._rng cimport rng
cimport microscopes.cxx.common._type_info_h as ti

cimport numpy as np

cdef class abstract_dataview:
    cdef dataview *_thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef np.ndarray _npd

cdef vector[ti.runtime_type_info] get_c_types(np.ndarray npd)
