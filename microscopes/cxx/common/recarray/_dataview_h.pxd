from libcpp.vector cimport vector
from libcpp cimport bool as cbool
from libc.stdint cimport uint8_t
from libc.stddef cimport size_t

from microscopes.cxx.common._random_fwd_h cimport rng_t
from microscopes.cxx.common._runtime_type_h cimport runtime_type

cdef extern from "microscopes/common/recarray/dataview.hpp" namespace "microscopes::common::recarray":
    cdef cppclass row_accessor:
        row_accessor()
        row_accessor(uint8_t *, cbool *, vector[runtime_type] *)
        cbool ismasked(size_t)
        const runtime_type & curtype()
        unsigned curshape()
        void bump()

    cdef cppclass row_mutator:
        row_mutator()
        row_mutator(uint8_t *, vector[runtime_type] *)
        void set(row_accessor &) except +
        void bump()

    cdef cppclass dataview:
        row_accessor get() except +
        vector[runtime_type] & types()
        void next()
        void reset()
        cbool end()

    cdef cppclass row_major_dataview(dataview):
        row_major_dataview(uint8_t *, cbool *, size_t, vector[runtime_type] &) except +
        void permute(rng_t &) except +
        void reset_permutation()

