cdef extern from "<memory>" namespace "std":
    cdef cppclass shared_ptr[T]:
        shared_ptr()
        shared_ptr(T *) except +
        T *get()
        void reset[U](U *) except +

    shared_ptr[T] static_pointer_cast[T, U] (const shared_ptr[U] &)
    shared_ptr[T] dynamic_pointer_cast[T, U] (const shared_ptr[U] &)
