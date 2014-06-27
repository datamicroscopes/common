from microscopes.cxx._models_h cimport distributions_factory
from microscopes.cxx._models_h cimport BetaBernoulli as c_bb
from microscopes.cxx._models_h cimport BetaNegativeBinomial as c_bnb
from microscopes.cxx._models_h cimport GammaPoisson as c_gp
from microscopes.cxx._models_h cimport NormalInverseChiSq as c_nich

cdef class factory:
    cdef shared_ptr[model] new_cmodel(self):
        # XXX: cython does not support virtual abstract classes
        raise Exception("Abstract class")

cdef class bb_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        cdef distributions_factory[c_bb] f = distributions_factory[c_bb]()
        return f.new_instance()

cdef class bnb_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        cdef distributions_factory[c_bnb] f = distributions_factory[c_bnb]()
        return f.new_instance()

cdef class gp_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        cdef distributions_factory[c_gp] f = distributions_factory[c_gp]()
        return f.new_instance()

cdef class nich_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        cdef distributions_factory[c_nich] f = distributions_factory[c_nich]()
        return f.new_instance()
