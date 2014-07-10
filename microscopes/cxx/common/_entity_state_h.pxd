cdef extern from "microscopes/common/entity_state.hpp" namespace "microscopes::common":
    cdef cppclass fixed_entity_based_state_object:
        pass

    cdef cppclass entity_based_state_object(fixed_entity_based_state_object):
        pass
