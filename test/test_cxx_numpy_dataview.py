from microscopes.common.recarray.dataview import (
    numpy_dataview as recarray_numpy_dataview,
)
from microscopes.common.relation.dataview import (
    numpy_dataview as relation_numpy_dataview,
)
from microscopes.common.rng import rng

import numpy as np
import numpy.ma as ma

from nose.tools import (
    assert_equals,
    assert_almost_equals,
    assert_list_equal,
    assert_is_not_none,
)


# XXX(stephentu): copied from kernel's test folder
def assert_1d_lists_almost_equals(first,
                                  second,
                                  places=None,
                                  msg=None,
                                  delta=None):
    assert_equals(len(first), len(second), msg=msg)
    for i, j in zip(first, second):
        assert_almost_equals(i, j, places=places, msg=msg, delta=delta)


def test_recarray_numpy_dataview():
    x = np.array([(False, 32.), (True, 943.), (False, -32.)],
                 dtype=[('', bool), ('', float)])
    view = recarray_numpy_dataview(x)
    assert_is_not_none(view)
    assert_equals(view.size(), x.shape[0])
    assert_equals(len(view), x.shape[0])

    for a, b in zip(x, view):
        assert_equals(a[0], b[0])
        assert_almost_equals(a[1], b[1])

    r = rng()
    acc = 0
    for a in view:
        acc += a[0]
    assert_equals(acc, sum(e[0] for e in x))


def test_recarray_numpy_dataview_subarray():
    x = np.array([(1, (2., 3.)), (-1, (-3., 54.))],
                 dtype=[('', np.int32), ('', np.float32, (2,))])
    view = recarray_numpy_dataview(x)
    assert_is_not_none(view)
    assert_equals(view.size(), x.shape[0])

    for a, b in zip(x, view):
        assert_equals(a[0], b[0])
        assert_equals(a[1].shape, b[1].shape)
        for f, g in zip(a[1], b[1]):
            assert_almost_equals(f, g)


def test_recarray_numpy_dataview_masked():
    x = ma.masked_array(
        np.array([(True, False, True, True, True)], dtype=[('', np.bool)] * 5),
        mask=[(False, False, True, True, True)])
    view = recarray_numpy_dataview(x)
    assert_is_not_none(view)
    assert_equals(view.size(), x.shape[0])

    for a, b in zip(x, view):
        assert_equals(a.mask, b.mask)
        for mask, (aval, bval) in zip(a.mask, zip(a, b)):
            if mask:
                continue
            assert_equals(aval, bval)


def test_recarray_numpy_dataview_pickle():
    import pickle

    # not masked, int32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1),],
                 dtype=[('', np.int32)] * 5)
    view = recarray_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_list_equal(list(v for v in view), list(v for v in view1))

    # not masked, float32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1),],
                 dtype=[('', np.float32)] * 5)
    view = recarray_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_1d_lists_almost_equals(
        list(v for v in view), list(v for v in view1))

    # masked, int32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1),],
                 dtype=[('', np.int32)] * 5)
    mask = [(False, False, False, False, True),
            (True, False, True, True, False)]
    y = ma.array(y, mask=mask)
    view = recarray_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    # XXX(stephentu): masked arrays suck and don't implement equality
    r = repr(list(v for v in view))
    r1 = repr(list(v for v in view1))
    assert_equals(r, r1)

    # masked, float32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1),],
                 dtype=[('', np.float32)] * 5)
    mask = [(False, False, False, False, False),
            (False, False, False, False, False)]
    y = ma.array(y, mask=mask)
    view = recarray_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_1d_lists_almost_equals(
        list(v.data for v in view),
        list(v.data for v in view1))


def test_relation_numpy_dataview():
    x = np.zeros((2, 3, 4), dtype=np.bool)
    view = relation_numpy_dataview(x)
    assert_is_not_none(view)
    assert_equals(view.shape(), (2, 3, 4))


def test_relation_numpy_dataview_masked():
    x = np.zeros((2, 3, 4), dtype=np.bool)
    x = ma.masked_array(x, mask=np.ones(x.shape))
    view = relation_numpy_dataview(x)
    assert_is_not_none(view)
    assert_equals(view.shape(), (2, 3, 4))
