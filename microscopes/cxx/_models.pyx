from microscopes.cxx._models_h cimport BetaBernoulli as c_bb, \
                                       BetaNegativeBinomial as c_bnb, \
                                       GammaPoisson as c_gp, \
                                       NormalInverseChiSq as c_nich, \
                                       DirichletDiscrete128 as c_dd, \
                                       distributions_model as c_distributions_model, \
                                       distributions_model_dd128 as c_distributions_model_dd128
from microscopes.cxx._bbnc_h cimport bbnc_model as c_bbnc
from microscopes.cxx._niw_h cimport niw_model as c_niw

cdef class _base:
    cdef shared_ptr[model] get(self):
        return self._thisptr
    cdef shared_ptr[hypers] create_hypers(self):
        return self._thisptr.get().create_hypers()

cdef class _bb(_base):
    def __cinit__(self):
        self._thisptr.reset(new c_distributions_model[c_bb]())

cdef class _bnb(_base):
    def __cinit__(self):
        self._thisptr.reset(new c_distributions_model[c_bnb]())

cdef class _gp(_base):
    def __cinit__(self):
        self._thisptr.reset(new c_distributions_model[c_gp]())

cdef class _nich(_base):
    def __cinit__(self):
        self._thisptr.reset(new c_distributions_model[c_nich]())

cdef class _dd(_base):
    def __cinit__(self, int size):
        self._thisptr.reset(new c_distributions_model_dd128(size))

cdef class _bbnc(_base):
    def __cinit__(self):
        self._thisptr.reset(new c_bbnc())

cdef class _niw(_base):
    def __cinit__(self, int dim):
        self._thisptr.reset(new c_niw(dim))
