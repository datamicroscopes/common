# cython: embedsignature=True


"""Dataviews for relation (tensor) data

These are the data inputs for the ``irm`` model.

"""

from microscopes.common.relation._dataview cimport (
    numpy_dataview as _numpy_dataview,
    sparse_2d_dataview as _sparse_2d_dataview,
)


class numpy_dataview(_numpy_dataview):
    """numpy_dataview(npd)

    A ``numpy.ndarray`` dataview.

    Parameters
    ----------
    npd : array

    Examples
    --------
    >>> Y = np.array([[1, 2], [3, 4]])
    >>> view = numpy_dataview(Y)
    >>> print view.shape
    (2, 2)

    """

class sparse_2d_dataview(_sparse_2d_dataview):
    """sparse_2d_dataview(rep)

    A ``scipy.sparse`` dataview

    Parameters
    ----------
    rep : a ``scipy.sparse`` matrix

    Examples
    --------
    >>> Y = scipy.sparse.coo_matrix((3, 4))
    >>> view = sparse_2d_dataview(Y)
    >>> print view.shape
    (3, 4)

    """
