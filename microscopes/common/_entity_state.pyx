# cython: embedsignature=True


from microscopes.models import model_descriptor
import numpy as np

cdef class entity_based_state_object:
    def __init__(self, models):
        for m in models:
            if not isinstance(m, model_descriptor):
                raise ValueError(
                    "expecting model_descriptor, got {}".format(repr(m)))
        self._models = list(models)

    # expose enough of the API here

    def assignments(self):
        return list(self._thisptr.get().assignments())

    def nentities(self):
        return self._thisptr.get().nentities()

    def ngroups(self):
        return self._thisptr.get().ngroups()

    def add_value(self, int gid, int eid, rng r):
        self._thisptr.get().add_value(gid, eid, r._thisptr[0])

    def remove_value(self, int eid, rng r):
        return self._thisptr.get().remove_value(eid, r._thisptr[0])

    def score_value(self, int eid, rng r):
        cdef pair[vector[size_t], vector[float]] ret = (
            self._thisptr.get().score_value(eid, r._thisptr[0])
        )
        return (
            [int(x) for x in ret.first],
            np.array([x for x in ret.second], dtype=np.float)
        )

    cdef c_entity_based_state_object * raw_px(self):
        return <c_entity_based_state_object *> self._thisptr.get()

    def empty_groups(self):
        return list(self.raw_px().empty_groups())

    def create_group(self, rng r):
        return self.raw_px().create_group(r._thisptr[0])

    def delete_group(self, int gid):
        self.raw_px().delete_group(gid)
