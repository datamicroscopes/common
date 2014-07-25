from libcpp.vector cimport vector
from libcpp.utility cimport pair
from libcpp.string cimport string
from libcpp cimport bool as cbool
from libc.stddef cimport size_t

from microscopes.cxx.common._type_info_h cimport primitive_type

cdef extern from "microscopes/common/runtime_type.hpp" namespace "microscopes::common":
    cdef cppclass runtime_type:
        runtime_type()
        runtime_type(primitive_type)
        runtime_type(primitive_type, unsigned)
        primitive_type t()
        unsigned n()
        cbool vec()
        string str() except +   

cdef extern from "microscopes/common/runtime_type.hpp" namespace "microscopes::common::runtime_type":
    cdef cppclass offsets_ret_t:
        offsets_ret_t()
        vector[size_t] offsets_
        size_t rowsize_
        size_t maskrowsize_

    offsets_ret_t GetOffsetsAndSize(vector[runtime_type] &) except +
