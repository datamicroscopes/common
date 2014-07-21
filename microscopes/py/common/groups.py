import numpy as np
from microscopes.io.schema_pb2 import \
    GroupManager as GroupManagerMessage, \
    GroupData as GroupDataMessage

class GroupManager(object):
    """
    Abstracts away a fixed set of N entities in a chinese restaurant process,
    each table having an opaque reference associated with it
    """

    NOT_ASSIGNED = -1

    def __init__(self, n):
        self._n = n
        self._assignments = -1*np.ones(self._n, dtype=np.int)
        self._gid = 0
        self._gdata = {}
        self._gempty = set() # to support fast empty group lookup

    def get_hp(self):
        return {'alpha':self._alpha}

    def set_hp(self, raw):
        self._alpha = float(raw['alpha'])

    def alpha(self):
        return self._alpha

    def nentities(self):
        return self._n

    def ngroups(self):
        return len(self._gdata)

    def groupsize(self, gid):
        return self._gdata[gid][0]

    def no_entities_assigned(self):
        """
        runtime: O(n)
        """
        return (self._assignments == -1).all()

    def all_entities_assigned(self):
        """
        runtime: O(n)
        """
        return not (self._assignments == -1).any()

    def empty_groups(self):
        return list(self._gempty)

    def group_data(self, gid):
        return self._gdata[gid][1]

    def groupiter(self):
        return self._gdata.iteritems()

    def assignments(self):
        return list(self._assignments)

    def create_group(self, gdata):
        """
        Creates the new group's groupid
        """
        gid = self._gid
        self._gid += 1
        self._gdata[gid] = [0, gdata]
        self._gempty.add(gid)
        return gid

    def delete_group(self, gid):
        """
        Can only do this if the group is empty
        """
        assert not self.groupsize(gid), 'group needs to be empty'
        assert gid in self._gempty
        del self._gdata[gid]
        self._gempty.remove(gid)

    def add_entity_to_group(self, gid, eid):
        """
        Add an unassigned entity to a group
        returns the group data
        """
        assert self._assignments[eid] == self.NOT_ASSIGNED
        self._assignments[eid] = gid
        ref = self._gdata[gid]
        cnt, gdata = ref
        if not cnt:
            self._gempty.remove(gid)
        ref[0] = cnt + 1
        return gdata

    def remove_entity_from_group(self, eid):
        """
        Remove the entity from its currently assigned group
        returns gid, gdata
        """
        assert self._assignments[eid] != self.NOT_ASSIGNED
        gid = self._assignments[eid]
        self._assignments[eid] = self.NOT_ASSIGNED
        ref = self._gdata[gid]
        cnt, gdata = ref
        if cnt == 1:
            assert gid not in self._gempty
            self._gempty.add(gid)
        ref[0] = cnt - 1
        return gid, gdata

    def score_assignment(self):
        """
        computes log p(C)
        """
        # CRP
        lg_sum = 0.0
        assignments = self._assignments
        counts = { assignments[0] : 1 }
        for i, ci in enumerate(assignments):
            if i == 0:
                continue
            assert ci != -1
            cnt = counts.get(ci, 0)
            numer = cnt if cnt else self._alpha
            denom = i + self._alpha
            lg_sum += np.log(numer / denom)
            counts[ci] = cnt + 1
        return lg_sum

    def serialize(self, group_serializer_fn):
        m = GroupManagerMessage()
        m.alpha = self._alpha
        for s in self._assignments:
            m.assignments.append(s)
        for gid, (_, gdata) in self._gdata.iteritems():
            g = m.groups.add()
            g.id = gid
            g.data = group_serializer_fn(gdata)
        return m.SerializeToString()

    @classmethod
    def deserialize(cls, s, group_deserializer_fn):
        m = GroupManagerMessage()
        m.ParseFromString(s)
        assignments = np.array(m.assignments, dtype=np.int)
        assert len(assignments), "empty assignment vector"
        counts = {}
        manager = cls(len(assignments))
        manager._assignments = assignments
        for gid in assignments:
            assert gid == cls.NOT_ASSIGNED or gid >= 0, "invalid gid"
            if gid == cls.NOT_ASSIGNED:
                continue
            counts[gid] = counts.get(gid, 0) + 1
        for g in m.groups:
            if g.id not in counts:
                manager._gempty.add(g.id)
            gdata = group_deserializer_fn(g.data)
            manager._gdata[g.id] = [counts.get(g.id, 0), gdata]
        manager._gid = max(manager._gdata.keys()) + 1
        return manager
