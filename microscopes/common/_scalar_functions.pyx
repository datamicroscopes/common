# cython: embedsignature=True


cdef class scalar_function:
    def __call__(self, *args):
        cdef vector[float] c_args
        for arg in args:
            c_args.push_back(float(arg))
        return self._func(c_args)

    def input_dim(self):
        return self._func.input_dim()
