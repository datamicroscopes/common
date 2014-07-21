from microscopes.py.common.groups import GroupManager

def test_group_manager_serializer():
    g = GroupManager(3)
    g.set_hp({'alpha':2.35})
    g.create_group([0])
    g.create_group([0])
    g.create_group([0])

    g.add_entity_to_group(0, 0)[0] += 1
    g.add_entity_to_group(0, 1)[0] += 1
    g.add_entity_to_group(1, 2)[0] += 1

    assert g.group_data(0)[0] == 2
    assert g.group_data(1)[0] == 1
    assert g.group_data(2)[0] == 0

    raw = g.serialize(lambda x: str(x[0]))
    g1 = GroupManager.deserialize(raw, lambda raw: [int(raw)])
    assert set(k for k, _ in g.groupiter()) == set(k for k, _ in g1.groupiter())
    assert set(g.empty_groups()) == set(g1.empty_groups())
    assert g.assignments() == g1.assignments()
    assert g.group_data(0) == g1.group_data(0)
    assert g.group_data(1) == g1.group_data(1)
    assert g.group_data(2) == g1.group_data(2)
