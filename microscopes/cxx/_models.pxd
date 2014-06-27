from microscopes.cxx._models_h cimport model
from microscopes._shared_ptr_h cimport shared_ptr

cdef class factory:
    cdef shared_ptr[model] new_cmodel(self)

cdef class bb_factory(factory):
    cdef shared_ptr[model] new_cmodel(self)

cdef class bnb_factory(factory):
    cdef shared_ptr[model] new_cmodel(self)

cdef class gp_factory(factory):
    cdef shared_ptr[model] new_cmodel(self)

cdef class nich_factory(factory):
    cdef shared_ptr[model] new_cmodel(self)
