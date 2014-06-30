from libcpp.string cimport string
cdef extern from "microscopes/common/typedefs.hpp" namespace "microscopes::common":
    ctypedef string hyperparam_bag_t
    ctypedef string suffstats_bag_t

    # really a typedef in our header, but we lie to cython here, since
    # cython doesn't support std::function<float(float)>
    cdef cppclass scalar_1d_float_fn:
        scalar_1d_float_fn()
        float operator()(float) except +
