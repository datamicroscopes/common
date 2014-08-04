from libcpp.vector cimport vector
from libcpp cimport bool as cbool
from libc.stdint cimport uint8_t, uint32_t
from libc.stddef cimport size_t

from microscopes.cxx.common._runtime_type_h cimport runtime_type

cdef extern from "microscopes/common/relation/dataview.hpp" namespace "microscopes::common::relation":
    cdef cppclass dataview:
        pass

    cdef cppclass row_major_dense_dataview(dataview):
        row_major_dense_dataview(uint8_t *, cbool *, const vector[size_t] &, const runtime_type &) except +

    cdef cppclass compressed_2darray(dataview):
        compressed_2darray(const uint8_t *,
                           const uint32_t *,
                           const uint32_t *,
                           const uint8_t *,
                           const uint32_t *,
                           const uint32_t *,
                           size_t,
                           size_t,
                           const runtime_type &) except +
