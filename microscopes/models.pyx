# XXX: we use the dbg versions for now, since the lp versions
# don't have protobuf_load()/protobuf_dump() defined for the groups

# Cython includes
from microscopes._models cimport (
    _bb, _bnb, _gp, _nich, _dd, _bbnc, _niw, _dm,
)

# Python includes

import numpy as np

from distributions.dbg.models import (
    bb as dbg_bb,
    bnb as dbg_bnb,
    gp as dbg_gp,
    nich as dbg_nich,
    dd as dbg_dd,
    niw as dbg_niw,
)

from distributions.io.schema_pb2 import (
    BetaBernoulli as pb_bb,
    BetaNegativeBinomial as pb_bnb,
    GammaPoisson as pb_gp,
    NormalInverseChiSq as pb_nich,
    DirichletDiscrete as pb_dd,
    NormalInverseWishart as pb_niw,
)

from microscopes.dbg.models import (
    bbnc as dbg_bbnc,
    dm as dbg_dm,
)

from microscopes.io.schema_pb2 import (
    BetaBernoulliNonConj as pb_bbnc,
    DirichletMultinomial as pb_dm,
)


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
        s.protobuf_dump(m)
        return m.SerializeToString()

    def shared_bytes_to_dict(self, raw):
        m = self._pb_type.Shared()
        m.ParseFromString(raw)
        s = self._model_module.Shared()
        s.protobuf_load(m)
        return s.dump()

    def group_dict_to_bytes(self, raw):
        s = self._model_module.Group()
        s.load(raw)
        m = self._pb_type.Group()
        s.protobuf_dump(m)
        return m.SerializeToString()

    def group_bytes_to_dict(self, raw):
        m = self._pb_type.Group()
        m.ParseFromString(raw)
        s = self._model_module.Group()
        s.protobuf_load(m)
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

bb = model_descriptor(
    py_descriptor=py_model(dbg_bb, pb_bb, dim=_scalar),
    c_descriptor=_bb(),
    default_params={'alpha': 1., 'beta': 1.})

bnb = model_descriptor(
    py_descriptor=py_model(dbg_bnb, pb_bnb, dim=_scalar),
    c_descriptor=_bnb(),
    default_params={'alpha': 1., 'beta': 1., 'r': 1})

gp = model_descriptor(
    py_descriptor=py_model(dbg_gp, pb_gp, dim=_scalar),
    c_descriptor=_gp(),
    default_params={'alpha': 1., 'inv_beta': 1.})

nich = model_descriptor(
    py_descriptor=py_model(dbg_nich, pb_nich, dim=_scalar),
    c_descriptor=_nich(),
    default_params={'mu': 0., 'kappa': 1., 'sigmasq': 1., 'nu': 1.})

dd = lambda size: model_descriptor(
    py_descriptor=py_model(dbg_dd, pb_dd, dim=_scalar),
    c_descriptor=_dd(size),
    default_params={'alphas': [1.] * size})

bbnc = model_descriptor(
    py_descriptor=py_model(dbg_bbnc, pb_bbnc, dim=_scalar),
    c_descriptor=_bbnc(),
    default_params=bb._default_params)

niw = lambda dim: model_descriptor(
    py_descriptor=py_model(dbg_niw, pb_niw, dim=dim),
    c_descriptor=_niw(dim),
    default_params={
        'mu': np.array([0.] * dim),
        'kappa': 1.0,
        'psi': np.eye(dim),
        'nu': float(dim),
    })

dm = lambda categories: model_descriptor(
    py_descriptor=py_model(dbg_dm, pb_dm, dim=categories),
    c_descriptor=_dm(categories),
    default_params=dd(categories)._default_params)
