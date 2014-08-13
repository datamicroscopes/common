# cython: embedsignature=True


from microscopes.common.recarray._dataview cimport \
    numpy_dataview as _numpy_dataview


class numpy_dataview(_numpy_dataview):
    pass
