from libcpp.vector cimport vector
from libc.stddef import size_t

from microscopes.common._runtime_type_h cimport runtime_type
from microscopes.common._typedefs_h cimport hyperparam_bag_t, suffstats_bag_t
from microscopes._shared_ptr_h cimport shared_ptr

cdef extern from "microscopes/models/base.hpp" namespace "microscopes::models":
    cdef cppclass group:
        suffstats_bag_t get_ss() except +
        void set_ss(const suffstats_bag_t &) except +

    cdef cppclass hypers:
        hyperparam_bag_t get_hp() except +
        void set_hp(const hyperparam_bag_t &) except +

    cdef cppclass model:
        shared_ptr[hypers] create_hypers() except +
        runtime_type get_runtime_type() except +

    ctypedef group* group_raw_ptr
    ctypedef shared_ptr[group] group_shared_ptr

    ctypedef hypers* hypers_raw_ptr
    ctypedef shared_ptr[hypers] hypers_shared_ptr

    ctypedef model* model_raw_ptr
    ctypedef shared_ptr[model] model_shared_ptr

cdef extern from "microscopes/models/distributions.hpp" namespace "microscopes::models":
    cdef cppclass distributions_model[T]:
        distributions_model()

    cdef cppclass distributions_model_dd128:
        distributions_model_dd128(unsigned) except +

    cdef cppclass distributions_model_niwv:
        distributions_model_niwv(unsigned) except +

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

cdef extern from "distributions/models/niw.hpp" namespace "distributions":
    cdef cppclass NormalInverseWishartV:
        pass

cdef extern from "microscopes/models/bbnc.hpp" namespace "microscopes::models":
    cdef cppclass bbnc_model:
        pass

cdef extern from "microscopes/models/dm.hpp" namespace "microscopes::models":
    cdef cppclass dm_model:
        dm_model(unsigned) except +
