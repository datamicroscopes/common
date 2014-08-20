"""Common helpers for model query interfaces.

Note you should generally be calling the model-specific query interfaces.

"""

import scipy.cluster
import numpy as np
import itertools as it

# Terminology used in this module:
# avec: a single assignment vector
# assignments: a list of avecs


def groups(avec, sort=False):
    """Turn an assignment vector in a clustering.

    Parameters
    ----------
    avec : assignment vector
    sort : bool, default False
        Whether or not the order of the clusters should be sorted by descending
        size (largest groups first)

    Returns
    -------
    clustering : a list of lists
        Note that ``len(clustering) == len(np.unique(avec))``

    """
    cluster_map = {}
    for idx, gid in enumerate(avec):
        lst = cluster_map.get(gid, [])
        lst.append(idx)
        cluster_map[gid] = lst
    if sort:
        return list(sorted(cluster_map.values(), key=len, reverse=True))
    else:
        return list(cluster_map.values())


def zmatrix(assignments):
    if not len(assignments):
        raise ValueError("empty assignments list")
    if len(set(map(len, assignments))) != 1:
        raise ValueError("assignment vectors should all be same size")

    # XXX(stephentu): should support sparse z-matrices for large n

    n = len(assignments[0])
    zmat = np.zeros((n, n), dtype=np.float32)
    for avec in assignments:
        clusters = groups(avec)
        for cluster in clusters:
            for i, j in it.product(cluster, repeat=2):
                zmat[i, j] += 1
    zmat /= float(len(assignments))
    return zmat


def _is_square_ndarray(n):
    return len(n.shape) == 2 and n.shape[0] == n.shape[1]


def _is_permutation(pi, n):
    if len(pi.shape) != 1 or pi.shape[0] != n:
        return False
    if not np.issubdtype(pi.dtype, np.integer):
        return False
    return len(np.unique(pi)) == n


def zmatrix_reorder(zmat, order):
    """Reorder a z-matrix given a permutation

    Parameters
    ----------
    zmat : (N, N) ndarray
    order : (N,) ndarray

    Returns
    -------
    reordered : (N, N) ndarray
        A new z-matrix with both columns and rows of `zmat` permuted
        according to `order`.

    """

    order = np.array(order, copy=False)

    if not _is_square_ndarray(zmat):
        raise ValueError("not a zmatrix")

    if not _is_permutation(order, zmat.shape[0]):
        raise ValueError("not a valid permutation")

    zmat = zmat[order]
    zmat = zmat[:, order]
    return zmat


def zmatrix_heuristic_block_ordering(zmat):
    """Heuristically generate a permutation of the axes of `zmat` which results
    in block-diagonal sub-matrices.

    Parameters
    ----------
    zmat : (N, N) ndarray

    Returns
    -------
    order : (N,)  ndarray
        A permutation on :math:`[N]` which heuristically generates the most
        block diagonal sub-matrices after re-ordering

    """

    if not _is_square_ndarray(zmat):
        raise ValueError("not a zmat")

    n = zmat.shape[0]
    zmat = np.array(zmat[np.triu_indices(n, k=1)])
    zmat = 1. - zmat
    l = scipy.cluster.hierarchy.linkage(zmat)
    return np.array(scipy.cluster.hierarchy.leaves_list(l))
