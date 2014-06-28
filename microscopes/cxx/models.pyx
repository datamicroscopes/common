# XXX: we use the dbg versions for now, since the lp versions 
# don't have load_protobuf()/dump_protobuf() defined for the groups

from distributions.dbg.models import bb as dbg_bb
from distributions.dbg.models import bnb as dbg_bnb
from distributions.dbg.models import gp as dbg_gp
from distributions.dbg.models import nich as dbg_nich

from distributions.io.schema_pb2 import BetaBernoulli as pb_bb
from distributions.io.schema_pb2 import BetaNegativeBinomial as pb_bnb
from distributions.io.schema_pb2 import GammaPoisson as pb_gp
from distributions.io.schema_pb2 import NormalInverseChiSq as pb_nich

from microscopes.cxx._models cimport bb_factory, bnb_factory, gp_factory, nich_factory

class py_model(object):
    def __init__(self, model_module, pb_type):
        self._model_module = model_module
        self._pb_type = pb_type

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

bb = (py_model(dbg_bb, pb_bb), bb_factory())
bnb = (py_model(dbg_bnb, pb_bnb), bnb_factory())
gp = (py_model(dbg_gp, pb_gp), gp_factory())
nich = (py_model(dbg_nich, pb_nich), nich_factory())
