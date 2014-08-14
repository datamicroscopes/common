# cython: embedsignature=True


"""Dataviews for data structured in records

These represent heterogenous data type feature vectors,
and are used as inputs to the ``mixturemodel`` model.
"""

from microscopes.common.recarray._dataview cimport (
    numpy_dataview as _numpy_dataview,
)

import numpy.ma as ma


class numpy_dataview(_numpy_dataview):
    """numpy_dataview(npd)

    A dataview around a numpy recarray. Supports masked arrays.

    Parameters
    ----------
    npd : array


    """

    def __reduce__(self):
        return (_reconstruct_numpy_dataview, (self._data, self._mask,))


def _reconstruct_numpy_dataview(data, mask):
    if mask is None:
        return numpy_dataview(data)
    else:
        return numpy_dataview(ma.array(data, mask=mask))
