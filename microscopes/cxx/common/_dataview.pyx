import numpy as np

TYPES = (
    ('bool'   , ti.TYPE_B)   ,
    ('int8'   , ti.TYPE_I8)  ,
    ('uint8'  , ti.TYPE_U8)  ,
    ('int16'  , ti.TYPE_I16) ,
    ('uint16' , ti.TYPE_U16) ,
    ('int32'  , ti.TYPE_I32) ,
    ('uint32' , ti.TYPE_U32) ,
    ('int64'  , ti.TYPE_I64) ,
    ('uint64' , ti.TYPE_U64) ,
    ('f4'     , ti.TYPE_F32) ,
    ('f8'     , ti.TYPE_F64) ,
)

def get_c_type(tpe):
    for name, ctype in TYPES:
        if np.dtype(name) == tpe:
            return ctype
    raise ValueError("Unknown type: " + tpe)

cdef vector[runtime_type] get_c_types(dtype):
    cdef vector[runtime_type] ctypes
    ctypes.reserve(len(dtype))
    for i in xrange(len(dtype)):
        if dtype[i].subdtype is None:
            # scalar field
            ctypes.push_back(runtime_type(get_c_type(dtype[i])))
        else:
            # vector field
            subdtype, shape = dtype[i].subdtype
            if len(shape) != 1:
                raise ValueError("unsupported shape: " + shape)
            ctypes.push_back(runtime_type(get_c_type(subdtype), shape[0]))
    return ctypes

cdef np.dtype get_np_type(const runtime_type & tpe):
    for name, ctype in TYPES:
        if tpe.t() != ctype:
            continue
        if tpe.vec():
            name = '({},){}'.format(tpe.n(), name)
        return np.dtype(name)
    raise Exception("unknown type: " + RuntimeTypeStr(tpe))
