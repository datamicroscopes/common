from microscopes.common.recarray.dataview import (
    numpy_dataview as recarray_numpy_dataview,
)
from microscopes.common.relation.dataview import (
    numpy_dataview as relation_numpy_dataview,
    sparse_2d_dataview,
)

import numpy as np
import numpy.ma as ma
import pickle
import itertools as it
import operator as op
from scipy.sparse import coo_matrix

from nose.tools import (
    assert_equals,
    assert_almost_equals,
    assert_list_equal,
    assert_is_not_none,
    assert_true,
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
    # not masked, int32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1)],
                 dtype=[('', np.int32)] * 5)
    view = recarray_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_list_equal(list(v for v in view), list(v for v in view1))

    # not masked, float32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1)],
                 dtype=[('', np.float32)] * 5)
    view = recarray_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_1d_lists_almost_equals(
        list(v for v in view), list(v for v in view1))

    # masked, int32
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1)],
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
    y = np.array([(1, 2, 3, 4, 5), (5, 4, 3, 2, 1)],
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


def test_relation_numpy_dataview_pickle():
    # not masked, int32
    y = np.random.randint(-10, 10, size=(10, 10))
    view = relation_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_true((y == view1.toarray()).all())

    # masked, int32
    y = ma.array(y, mask=(np.random.uniform(size=(10, 10)) < 0.5))
    view = relation_numpy_dataview(y)
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_true((y == view1.toarray()).all())


def test_relation_sparse_2d_dataview_pickle():

    def sparsify(y, fn=lambda x: x):
        inds = it.product(range(y.shape[0]), range(y.shape[1]))
        ijv = [(i, j, y[i, j]) for i, j in inds if fn(y[i, j])]
        args = (
            map(op.itemgetter(2), ijv),
            (map(op.itemgetter(0), ijv), map(op.itemgetter(1), ijv))
        )
        return coo_matrix(args, shape=y.shape)

    y = np.random.randint(-2, 2, size=(10, 10))
    view = sparse_2d_dataview(sparsify(y))
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_equals((view.tocsr() != view1.tocsr()).nnz, 0)

    y = np.random.uniform(size=(4, 3))
    view = sparse_2d_dataview(sparsify(y, fn=lambda x: x >= 0.4 and x <= 0.6))
    bstr = pickle.dumps(view)
    view1 = pickle.loads(bstr)
    assert_almost_equals(
        np.abs((view.tocsr() - view1.tocsr()).todense()).max(), 0., places=3)
