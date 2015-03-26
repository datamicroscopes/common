from libcpp.vector cimport vector
from libcpp.utility cimport pair

from microscopes._shared_ptr_h cimport shared_ptr, static_pointer_cast
from microscopes.common._entity_state_h cimport \
        entity_based_state_object as c_entity_based_state_object
from microscopes.common._rng cimport rng


cdef class entity_based_state_object:
    cdef c_entity_based_state_object * raw_px(self)
    cdef shared_ptr[c_entity_based_state_object] _thisptr
    cdef public list _models # list of model_descriptor objects
    cdef object _refs # used to hold references
