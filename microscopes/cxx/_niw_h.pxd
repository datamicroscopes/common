from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx._models_h cimport model

cdef extern from "microscopes/models/niw.hpp" namespace "microscopes::models":
    cdef cppclass niw_model:
        niw_model(unsigned) except +
