# cython: embedsignature=True


"""Dataviews for data structured with variadic feature size.

These represent homogeneous data type, variable length feature vectors, and are
used as inputs to the ``lda`` model.
"""

from microscopes.common.variadic._dataview cimport (
    numpy_dataview as _numpy_dataview,
)


class numpy_dataview(_numpy_dataview):
    """numpy_dataview(data)

    Parameters
    ----------
    data : list of 1-D array-like objects

    """

    def __reduce__(self):
        return (_reconstruct_numpy_dataview, (self._data,))


def _reconstruct_numpy_dataview(data):
    return numpy_dataview(data)
