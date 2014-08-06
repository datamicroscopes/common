from libcpp.vector cimport vector
from libcpp.utility cimport pair
from libc.stddef cimport size_t

from microscopes.common._random_fwd_h cimport rng_t

cdef extern from "microscopes/common/entity_state.hpp" namespace "microscopes::common":
    # expose enough of the API here

    cdef cppclass fixed_entity_based_state_object:
        vector[ssize_t] assignments() except +
        size_t nentities()
        size_t ngroups()

        void add_value(size_t, size_t, rng_t &) except +
        size_t remove_value(size_t, rng_t &) except +
        pair[vector[size_t], vector[float]] score_value(size_t, rng_t &) except +

    cdef cppclass entity_based_state_object(fixed_entity_based_state_object):
        vector[size_t] empty_groups() except +
        size_t create_group(rng_t &) except +
        void delete_group(size_t) except +
