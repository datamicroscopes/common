from libcpp.vector cimport vector

cdef extern from "microscopes/common/entity_state.hpp" namespace "microscopes::common":
    # expose enough of the API here

    cdef cppclass fixed_entity_based_state_object:
        vector[ssize_t] assignments() except +

    cdef cppclass entity_based_state_object(fixed_entity_based_state_object):
        pass
