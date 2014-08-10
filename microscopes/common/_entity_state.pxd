from libcpp.vector cimport vector
from libcpp.utility cimport pair

from microscopes._shared_ptr_h cimport shared_ptr, static_pointer_cast
from microscopes.common._entity_state_h cimport \
        fixed_entity_based_state_object as c_fixed_entity_based_state_object, \
        entity_based_state_object as c_entity_based_state_object
from microscopes.common._rng cimport rng

cdef class fixed_entity_based_state_object:
    cdef shared_ptr[c_fixed_entity_based_state_object] _thisptr
    cdef void set_fixed(self, const shared_ptr[c_fixed_entity_based_state_object] &o)
    cdef public list _models # list of model_descriptor objects
    cdef object _refs # used to hold references

cdef class entity_based_state_object(fixed_entity_based_state_object):
    cdef void set_fixed(self, const shared_ptr[c_fixed_entity_based_state_object] &o)
    cdef void set_entity(self, const shared_ptr[c_entity_based_state_object] &o)
    cdef shared_ptr[c_entity_based_state_object] px(self)
    cdef c_entity_based_state_object * raw_px(self)
