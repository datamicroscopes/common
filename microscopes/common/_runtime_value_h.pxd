cdef extern from "microscopes/common/runtime_value.hpp" namespace "microscopes::common":
    cdef cppclass value_accessor:
        value_accessor()

    cdef cppclass value_mutator:
        value_mutator()
