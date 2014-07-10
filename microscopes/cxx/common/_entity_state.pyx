cdef class fixed_entity_based_state_object:
    def __cinit__(self):
        pass

    cdef void set_fixed(self, const shared_ptr[c_fixed_entity_based_state_object] &o):
        self._thisptr = o

cdef class entity_based_state_object(fixed_entity_based_state_object):
    def __cinit__(self):
        pass

    cdef void set_fixed(self, const shared_ptr[c_fixed_entity_based_state_object] &o):
        # XXX: hacky
        raise Exception("no evidence of derived class");

    cdef void set_entity(self, const shared_ptr[c_entity_based_state_object] &o):
        # Cython's type system is too weak to allow us to express this
        # without the explicit pointer cast (in C++ this would be completely un-necessary)
        self._thisptr = static_pointer_cast[c_fixed_entity_based_state_object, c_entity_based_state_object](o)
