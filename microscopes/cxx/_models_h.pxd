from libcpp.string cimport string
from libcpp.vector cimport vector

from microscopes.cxx.common._runtime_type_h cimport runtime_type
from microscopes.cxx.common._typedefs_h cimport hyperparam_bag_t, suffstats_bag_t
from microscopes._shared_ptr_h cimport shared_ptr

cdef extern from "microscopes/models/base.hpp" namespace "microscopes::models":
    cdef cppclass model:
        hyperparam_bag_t get_hp() except +
        void set_hp(hyperparam_bag_t &) except +
        vector[runtime_type] get_runtime_type() except +
    ctypedef model* model_raw_ptr
    ctypedef shared_ptr[model] model_shared_ptr

cdef extern from "microscopes/models/distributions.hpp" namespace "microscopes::models":
    cdef cppclass distributions_factory[T]:
        distributions_factory()
        shared_ptr[model] new_instance() except +

cdef extern from "distributions/models/bb.hpp" namespace "distributions":
    cdef cppclass BetaBernoulli:
        pass

cdef extern from "distributions/models/bnb.hpp" namespace "distributions":
    cdef cppclass BetaNegativeBinomial:
        pass

cdef extern from "distributions/models/gp.hpp" namespace "distributions":
    cdef cppclass GammaPoisson:
        pass

cdef extern from "distributions/models/nich.hpp" namespace "distributions":
    cdef cppclass NormalInverseChiSq:
        pass

cdef extern from "microscopes/models/distributions.hpp" namespace "distributions":
    cdef cppclass DirichletDiscrete128:
        pass
