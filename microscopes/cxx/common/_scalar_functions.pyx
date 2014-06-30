cdef class scalar_function:
    def __call__(self, float arg0):
        return self._func(arg0)
