# cython: embedsignature=True


from microscopes.common.relation._dataview cimport \
    numpy_dataview as _numpy_dataview, \
    sparse_2d_dataview as _sparse_2d_dataview


class numpy_dataview(_numpy_dataview):
    pass


class sparse_2d_dataview(_sparse_2d_dataview):
    pass
