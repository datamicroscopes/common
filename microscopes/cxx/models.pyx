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

class py_model(object):
    def __init__(self, model_module, pb_type):
        self._model_module = model_module
        self._pb_type = pb_type

    def get_py_type(self):
        return self._model_module

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
    def __init__(self, py_descriptor, c_descriptor):
        self._py_descriptor = py_descriptor
        self._c_descriptor = c_descriptor

bb   = model_descriptor(py_model(dbg_bb, pb_bb), _bb())
bnb  = model_descriptor(py_model(dbg_bnb, pb_bnb), _bnb())
gp   = model_descriptor(py_model(dbg_gp, pb_gp), _gp())
nich = model_descriptor(py_model(dbg_nich, pb_nich), _nich())
dd   = lambda size: model_descriptor(py_model(dbg_dd, pb_dd), _dd(size))
bbnc = model_descriptor(py_model(py_bbnc, pb_bbnc), _bbnc())
niw  = lambda dim: model_descriptor(py_model(py_niw, pb_niw), _niw(dim))

def bbnc_create_feature_group_invocations():
    return int(CreateFeatureGroupInvocations())
