from microscopes.cxx.common.recarray._dataview_h cimport dataview
from microscopes.cxx.common._runtime_type_h cimport runtime_type

cimport numpy as np

cdef class abstract_dataview:
    cdef dataview *_thisptr

cdef class numpy_dataview(abstract_dataview):
    cdef np.ndarray _data
    cdef np.ndarray _mask
