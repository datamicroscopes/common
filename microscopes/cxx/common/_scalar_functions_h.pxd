from libcpp.vector cimport vector

cdef extern from "microscopes/common/scalar_functions.hpp" namespace "microscopes::common":
    cdef cppclass scalar_fn:
        float operator()(const vector[float] &) except +

    scalar_fn log_exponential(float)
    scalar_fn log_normal(float, float)
    scalar_fn log_noninformative_beta_prior()
