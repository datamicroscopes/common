from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx._models_h cimport model

cdef extern from "microscopes/models/niw.hpp" namespace "microscopes::models":
    cdef cppclass niw_model:
        pass

cdef extern from "microscopes/models/niw.hpp" namespace "microscopes::models::niw_model":
    shared_ptr[model] new_instance() except +
