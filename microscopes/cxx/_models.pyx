from microscopes.cxx._models_h cimport distributions_factory, \
                                       BetaBernoulli as c_bb, \
                                       BetaNegativeBinomial as c_bnb, \
                                       GammaPoisson as c_gp, \
                                       NormalInverseChiSq as c_nich, \
                                       DirichletDiscrete128 as c_dd
from microscopes.cxx._bbnc_h cimport new_instance as bbnc_new_instance
from microscopes.cxx._niw_h cimport new_instance as niw_new_instance

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

cdef class dd_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        cdef distributions_factory[c_dd] f = distributions_factory[c_dd]()
        return f.new_instance()

cdef class bbnc_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        return bbnc_new_instance()

cdef class niw_factory(factory):
    cdef shared_ptr[model] new_cmodel(self):
        return niw_new_instance()
