cdef extern from "eigen3/Eigen/Dense" namespace "Eigen":
    cdef cppclass VectorXf:
        VectorXf()
        VectorXf(int) except +
        int size()
        float & operator[](int) except +

    cdef cppclass MatrixXf:
        MatrixXf()
        MatrixXf(int, int) except +
        int rows()
        int cols()
