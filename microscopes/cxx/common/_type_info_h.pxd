cdef extern from "microscopes/common/type_info.h":
    enum _runtime_type_info:
       TYPE_INFO_B
       TYPE_INFO_I8
       TYPE_INFO_I16
       TYPE_INFO_I32
       TYPE_INFO_I64
       TYPE_INFO_F32
       TYPE_INFO_F64
    ctypedef _runtime_type_info runtime_type_info
