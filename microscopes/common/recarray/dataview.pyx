# cython: embedsignature=True


"""Dataviews for data structured in records

These represent heterogenous data type feature vectors,
and are used as inputs to the ``mixturemodel`` model.
"""

from microscopes.common.recarray._dataview cimport (
    numpy_dataview as _numpy_dataview,
)


class numpy_dataview(_numpy_dataview):
    """numpy_dataview(npd)

    A dataview around a numpy recarray

    Parameters
    ----------
    npd : array


    """
