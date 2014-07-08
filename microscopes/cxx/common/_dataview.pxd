from libcpp.vector cimport vector
from libc.stdint cimport uint8_t

cimport microscopes.cxx.common._type_info_h as ti
from microscopes.cxx.common._runtime_type_h cimport runtime_type, RuntimeTypeStr

cimport numpy as np

cdef vector[runtime_type] get_c_types(dtype)
cdef np.dtype get_np_type(const runtime_type &)
