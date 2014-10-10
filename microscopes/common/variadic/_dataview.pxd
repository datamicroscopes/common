from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.common._dataview cimport get_c_type
from microscopes.common.variadic._dataview_h cimport (
    dataview,
    row_major_dataview,
)
from microscopes.common._runtime_type_h cimport runtime_type

cimport numpy as np


cdef class abstract_dataview:
    cdef shared_ptr[dataview] _thisptr


cdef class numpy_dataview(abstract_dataview):
    cdef readonly list _data
