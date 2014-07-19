from microscopes.cxx._models_h cimport model, hypers
from microscopes._shared_ptr_h cimport shared_ptr

cdef class _base:
    cdef shared_ptr[model] _thisptr
    cdef shared_ptr[model] get(self)
    cdef shared_ptr[hypers] create_hypers(self)

cdef class _bb(_base):
    pass

cdef class _bnb(_base):
    pass

cdef class _gp(_base):
    pass

cdef class _nich(_base):
    pass

cdef class _dd(_base):
    pass

cdef class _bbnc(_base):
    pass

cdef class _niw(_base):
    pass
