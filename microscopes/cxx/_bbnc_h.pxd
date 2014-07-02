from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx._models_h cimport model

cdef extern from "microscopes/models/bbnc.hpp" namespace "microscopes::models":
    cdef cppclass bbnc_model:
        pass

cdef extern from "microscopes/models/bbnc.hpp" namespace "microscopes::models::bbnc_model":
    shared_ptr[model] new_instance() except +
