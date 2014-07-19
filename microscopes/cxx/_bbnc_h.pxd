from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx._models_h cimport model
from libc.stddef import size_t

cdef extern from "microscopes/models/bbnc.hpp" namespace "microscopes::models":
    cdef cppclass bbnc_model:
        pass

cdef extern from "microscopes/models/bbnc.hpp" namespace "microscopes::models::bbnc_hypers":
    size_t CreateFeatureGroupInvocations()
