from libcpp.vector cimport vector
from libcpp cimport bool as cbool
from libc.stdint cimport uint8_t
from libc.stddef cimport size_t

from microscopes.cxx.common._random_fwd_h cimport rng_t
from microscopes.cxx.common._type_info_h cimport runtime_type_info

cdef extern from "microscopes/common/dataview.hpp" namespace "microscopes::common":
    cdef cppclass row_accessor:
        row_accessor(uint8_t *, cbool *, vector[runtime_type_info] *, vector[size_t] *)

    cdef cppclass row_mutator:
        row_mutator(uint8_t *, vector[runtime_type_info] *, vector[size_t] *)

    cdef cppclass dataview:
        pass

    cdef cppclass row_major_dataview(dataview):
        row_major_dataview(uint8_t *, cbool *, size_t, vector[runtime_type_info] &) except +
        void permute(rng_t &) except +
        void reset_permutation() 
