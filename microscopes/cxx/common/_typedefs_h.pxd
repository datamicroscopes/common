from libcpp.string cimport string
from libcpp.vector cimport vector
cdef extern from "microscopes/common/typedefs.hpp" namespace "microscopes::common":
    ctypedef string hyperparam_bag_t
    ctypedef string suffstats_bag_t

    # really a typedef in our header, but we lie to cython here, since
    # cython doesn't support std::function<float(const vector<float>&)>
    cdef cppclass scalar_fn:
        scalar_fn()
        float operator()(const vector[float] &) except +
