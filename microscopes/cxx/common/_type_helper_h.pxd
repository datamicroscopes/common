from libcpp.vector cimport vector
from libcpp.utility cimport pair
from libcpp.string cimport string
from libcpp cimport bool as cbool
from libc.stddef cimport size_t

from microscopes.cxx.common._type_info_h cimport primitive_type

cdef extern from "microscopes/common/type_helper.hpp" namespace "microscopes::common":
    cdef cppclass runtime_type:
        runtime_type()
        runtime_type(primitive_type)
        runtime_type(primitive_type, unsigned)
        primitive_type t()
        unsigned n()
        cbool vec()

cdef extern from "microscopes/common/type_helper.hpp" namespace "microscopes::common::runtime_type_traits":
    pair[vector[size_t], size_t] GetOffsetsAndSize(vector[runtime_type] &) except +
    string RuntimeTypeStr(runtime_type &) except +
