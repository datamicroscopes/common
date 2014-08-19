# XXX(stephentu): these tests are more sanity checks (e.g. check for crashing)
# than actual unit tests

import numpy as np
from microscopes.common import query

from nose.tools import (
    assert_equals,
    assert_true,
    assert_almost_equals,
)


def test_groups():
    avec = [34, 34, 5, 11, 5, 5433]
    truth_clustering = [[0, 1], [2, 4], [3], [5]]
    clustering = query.groups(avec)
    assert_equals(len(clustering), len(np.unique(avec)))
    assert_equals(set(map(frozenset, truth_clustering)),
                  set(map(frozenset, clustering)))


def test_zmatrix():
    assignments = [
        [2, 345, 2, 2],
        [3, 345, 2, 2],
        [3, 9, 1, 1],
    ]
    zmat = query.zmatrix(assignments)
    assert_true(query._is_square_ndarray(zmat))


def test_zmatrix_reorder():
    assignments = [
        [2, 345, 2, 2],
        [3, 345, 2, 2],
        [3, 9, 1, 1],
    ]
    zmat = query.zmatrix(assignments)
    reordered = query.zmatrix_reorder(zmat, np.arange(zmat.shape[0]))
    assert_almost_equals(np.abs(zmat - reordered).max(), 0.)


def test_zmatrix_heuristic_block_reordering():
    assignments = [
        [2, 345, 2, 2],
        [3, 345, 2, 2],
        [3, 9, 1, 1],
    ]
    zmat = query.zmatrix(assignments)
    order = query.zmatrix_heuristic_block_ordering(zmat)
    assert_true(query._is_permutation(order, zmat.shape[0]))
