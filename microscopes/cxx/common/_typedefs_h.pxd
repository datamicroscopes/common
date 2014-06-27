from libcpp.string cimport string
cdef extern from "microscopes/common/typedefs.hpp" namespace "microscopes::common":
    ctypedef string hyperparam_bag_t 
    ctypedef string suffstats_bag_t
