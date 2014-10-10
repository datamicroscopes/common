from libcpp.vector cimport vector
from libc.stdint cimport uint8_t
from libc.stddef cimport size_t

from microscopes.common._runtime_type_h cimport runtime_type


cdef extern from "microscopes/common/variadic/dataview.hpp" namespace "microscopes::common::variadic":

    cdef cppclass row_accessor:
        pass

    cdef cppclass dataview:
        row_accessor get(size_t) except +
        size_t rowsize(size_t) except +
        size_t size
        const runtime_type & type() except +

    cdef cppclass row_major_dataview(dataview):
        row_major_dataview(const vector[const uint8_t *] &,
                           const vector[unsigned] &,
                           const runtime_type &) except +
