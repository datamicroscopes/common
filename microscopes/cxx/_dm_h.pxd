from microscopes._shared_ptr_h cimport shared_ptr
from microscopes.cxx._models_h cimport model

cdef extern from "microscopes/models/dm.hpp" namespace "microscopes::models":
    cdef cppclass dm_model:
        dm_model(unsigned) except +
