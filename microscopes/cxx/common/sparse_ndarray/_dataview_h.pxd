from libcpp.vector cimport vector
from libcpp cimport bool as cbool
from libc.stdint cimport uint8_t
from libc.stddef cimport size_t

from microscopes.cxx.common._runtime_type_h cimport runtime_type

cdef extern from "microscopes/common/sparse_ndarray/dataview.hpp" namespace "microscopes::common::sparse_ndarray":
    cdef cppclass dataview:
        pass

    cdef cppclass row_major_dense_dataview(dataview):
        row_major_dense_dataview(uint8_t *, cbool *, const vector[size_t] &, const runtime_type &) except +

