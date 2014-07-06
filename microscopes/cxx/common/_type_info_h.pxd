cdef extern from "microscopes/common/type_info.h":
    enum _primitive_type:
       TYPE_B
       TYPE_I8
       TYPE_U8
       TYPE_I16
       TYPE_U16
       TYPE_I32
       TYPE_U32
       TYPE_I64
       TYPE_U64
       TYPE_F32
       TYPE_F64
    ctypedef _primitive_type primitive_type
