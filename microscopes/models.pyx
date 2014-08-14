# cython: embedsignature=True


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

from microscopes.common.scalar_functions import (
    log_exponential,
    log_noninformative_beta_prior,
)
from microscopes.common import validator


class py_model(object):

    def __init__(self, model_module, pb_module, dtype=None):
        self._model_module = model_module
        self._pb_module = pb_module
        if dtype is None:
            self._dtype = np.dtype(self._model_module.Value)
        else:
            validator.validate_type(dtype, np.dtype, param_name='dtype')
            self._dtype = dtype

    def get_np_dtype(self):
        return self._dtype

    def shared_dict_to_bytes(self, raw):
        s = self._model_module.Shared()
        s.load(raw)
        m = self._pb_module.Shared()
        s.protobuf_dump(m)
        return m.SerializeToString()

    def shared_bytes_to_dict(self, raw):
        m = self._pb_module.Shared()
        m.ParseFromString(raw)
        s = self._model_module.Shared()
        s.protobuf_load(m)
        return s.dump()

    def group_dict_to_bytes(self, raw):
        s = self._model_module.Group()
        s.load(raw)
        m = self._pb_module.Group()
        s.protobuf_dump(m)
        return m.SerializeToString()

    def group_bytes_to_dict(self, raw):
        m = self._pb_module.Group()
        m.ParseFromString(raw)
        s = self._model_module.Group()
        s.protobuf_load(m)
        return s.dump()


class model_descriptor(object):

    def __init__(self,
                 name,
                 py_descriptor,
                 c_descriptor,
                 default_hyperparams,
                 default_hyperpriors):
        self._name = name
        self._py_descriptor = py_descriptor
        self._c_descriptor = c_descriptor
        self._default_hyperparams = default_hyperparams
        self._default_hyperpriors = default_hyperpriors

    def name(self):
        return self._name

    def py_desc(self):
        return self._py_descriptor

    def c_desc(self):
        return self._c_descriptor

    def default_hyperparams(self):
        return self._default_hyperparams

    def default_hyperpriors(self):
        return self._default_hyperpriors

    def _param(self):
        name = self.name()
        if name in ('dd', 'dm'):
            return len(self._default_hyperparams['alphas'])
        elif name == 'niw':
            return len(self._default_hyperparams['mu'])
        else:
            return None

    def __reduce__(self):
        return (_reconstruct_model_descriptor, (self._name, self._param()))


def _reconstruct_model_descriptor(name, param):
    desc = globals()[name]
    if param is None:
        return desc
    else:
        return desc(param)


# XXX(stephentu): someone should check that the default hyperpriors listed here
# are sane and useful!


bb = model_descriptor(
    name='bb',
    py_descriptor=py_model(dbg_bb, pb_bb),
    c_descriptor=_bb(),
    default_hyperparams={'alpha': 1., 'beta': 1.},
    default_hyperpriors={
        ('alpha', 'beta') : log_noninformative_beta_prior,
    })


bnb = model_descriptor(
    name='bnb',
    py_descriptor=py_model(dbg_bnb, pb_bnb),
    c_descriptor=_bnb(),
    default_hyperparams={'alpha': 1., 'beta': 1., 'r': 1},
    default_hyperpriors={
        ('alpha', 'beta') : log_noninformative_beta_prior,
    })


gp = model_descriptor(
    name='gp',
    py_descriptor=py_model(dbg_gp, pb_gp),
    c_descriptor=_gp(),
    default_hyperparams={'alpha': 1., 'inv_beta': 1.},
    default_hyperpriors={
        'alpha': log_exponential(1.),
        'inv_beta': log_exponential(1.),
    })


nich = model_descriptor(
    name='nich',
    py_descriptor=py_model(dbg_nich, pb_nich),
    c_descriptor=_nich(),
    default_hyperparams={'mu': 0., 'kappa': 1., 'sigmasq': 1., 'nu': 1.},
    default_hyperpriors={
        # XXX(stephentu): put something sane here
    })


def dd(size):
    validator.validate_positive(size, param_name='size')
    desc = model_descriptor(
        name='dd',
        py_descriptor=py_model(dbg_dd, pb_dd),
        c_descriptor=_dd(size),
        default_hyperparams={'alphas': [1.] * size},
        default_hyperpriors={
            # XXX(stephentu): put something sane here
        })
    return desc


bbnc = model_descriptor(
    name='bbnc',
    py_descriptor=py_model(dbg_bbnc, pb_bbnc),
    c_descriptor=_bbnc(),
    default_hyperparams=bb._default_hyperparams,
    default_hyperpriors=bb._default_hyperpriors)


def niw(dim):
    validator.validate_positive(dim, param_name='dim')
    dtype = np.dtype((float, (dim,)))
    desc = model_descriptor(
        name='niw',
        py_descriptor=py_model(dbg_niw, pb_niw, dtype=dtype),
        c_descriptor=_niw(dim),
        default_hyperparams={
            'mu': np.array([0.] * dim),
            'kappa': 1.0,
            'psi': np.eye(dim),
            'nu': float(dim),
        },
        default_hyperpriors={
            # XXX(stephentu): put something sane here
        })
    return desc


def dm(categories):
    validator.validate_positive(categories, param_name='categories')
    dtype = np.dtype((dbg_dm.Value, (categories,)))
    py_descriptor = py_model(dbg_dm, pb_dm, dtype=dtype)
    desc = model_descriptor(
        name='dm',
        py_descriptor=py_descriptor,
        c_descriptor=_dm(categories),
        default_hyperparams=dd(categories)._default_hyperparams,
        default_hyperpriors=dd(categories)._default_hyperpriors)
    return desc
