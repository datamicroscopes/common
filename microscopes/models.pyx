# XXX: we use the dbg versions for now, since the lp versions
# don't have load_protobuf()/dump_protobuf() defined for the groups

from distributions.dbg.models import bb as dbg_bb, \
                                     bnb as dbg_bnb, \
                                     gp as dbg_gp, \
                                     nich as dbg_nich, \
                                     dd as dbg_dd

from distributions.io.schema_pb2 import BetaBernoulli as pb_bb, \
                                        BetaNegativeBinomial as pb_bnb, \
                                        GammaPoisson as pb_gp, \
                                        NormalInverseChiSq as pb_nich, \
                                        DirichletDiscrete as pb_dd

from microscopes.cxx._models cimport \
  _bb, _bnb, _gp, _nich, _dd, _bbnc, _niw

from microscopes.py.models import bbnc as py_bbnc, \
                                  niw as py_niw

from microscopes.io.schema_pb2 import BetaBernoulliNonConj as pb_bbnc, \
                                      NormalInverseWishart as pb_niw

from microscopes.cxx._bbnc_h cimport CreateFeatureGroupInvocations

import numpy as np

class py_model(object):
    def __init__(self, model_module, pb_type, dim):
        self._model_module = model_module
        self._pb_type = pb_type
        self._dim = dim

    def get_np_dtype(self):
        if self._dim is None:
            # we tread None dim as scalar
            return np.dtype(self._model_module.Value)
        else:
            return np.dtype((self._model_module.Value, (self._dim,)))

    def shared_dict_to_bytes(self, raw):
        s = self._model_module.Shared()
        s.load(raw)
        m = self._pb_type.Shared()
        s.dump_protobuf(m)
        return m.SerializeToString()

    def shared_bytes_to_dict(self, raw):
        m = self._pb_type.Shared()
        m.ParseFromString(raw)
        s = self._model_module.Shared()
        s.load_protobuf(m)
        return s.dump()

    def group_dict_to_bytes(self, raw):
        s = self._model_module.Group()
        s.load(raw)
        m = self._pb_type.Group()
        s.dump_protobuf(m)
        return m.SerializeToString()

    def group_bytes_to_dict(self, raw):
        m = self._pb_type.Group()
        m.ParseFromString(raw)
        s = self._model_module.Group()
        s.load_protobuf(m)
        return s.dump()

class model_descriptor(object):
    def __init__(self, py_descriptor, c_descriptor, default_params):
        self._py_descriptor = py_descriptor
        self._c_descriptor = c_descriptor
        self._default_params = default_params

    def py_desc(self):
        return self._py_descriptor

    def c_desc(self):
        return self._c_descriptor

    def default_params(self):
        return self._default_params

_scalar = None

bb   = model_descriptor(
        py_descriptor=py_model(dbg_bb, pb_bb, dim=_scalar), 
        c_descriptor=_bb(), 
        default_params={'alpha':1.,'beta':1.})

bnb  = model_descriptor(
        py_descriptor=py_model(dbg_bnb, pb_bnb, dim=_scalar), 
        c_descriptor=_bnb(), 
        default_params={'alpha':1.,'beta':1.,'r':1})

gp   = model_descriptor(
        py_descriptor=py_model(dbg_gp, pb_gp, dim=_scalar), 
        c_descriptor=_gp(), 
        default_params={'alpha':1.,'inv_beta':1.})

nich = model_descriptor(
        py_descriptor=py_model(dbg_nich, pb_nich, dim=_scalar), 
        c_descriptor=_nich(), 
        default_params={'mu':0.,'kappa':1.,'sigmasq':1.,'nu':1.})

dd   = lambda size: model_descriptor(
        py_descriptor=py_model(dbg_dd, pb_dd, dim=_scalar), 
        c_descriptor=_dd(size), 
        default_params={'alphas':[1.]*size})

bbnc = model_descriptor(
        py_descriptor=py_model(py_bbnc, pb_bbnc, dim=_scalar), 
        c_descriptor=_bbnc(),
        default_params=bb._default_params)

def _raise_unimplemented():
    raise RuntimeError("not implemented")
# XXX: default params
niw  = lambda dim: model_descriptor(
        py_descriptor=py_model(py_niw, pb_niw, dim=dim), 
        c_descriptor=_niw(dim), 
        default_params=_raise_unimplemented())

def bbnc_create_feature_group_invocations():
    return int(CreateFeatureGroupInvocations())
