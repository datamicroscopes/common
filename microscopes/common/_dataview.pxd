from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

from microscopes.common._runtime_type_h cimport runtime_type
cimport microscopes.common._type_info_h as ti

cimport numpy as np

cdef runtime_type get_c_type(dtype)
cdef vector[runtime_type] get_c_types(dtype)
cdef np.dtype get_np_type(const runtime_type &)
