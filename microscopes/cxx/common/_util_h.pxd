from microscopes._eigen_h cimport MatrixXf

cdef extern from "microscopes/common/util.hpp" namespace "microscopes::common::util":
    void set(MatrixXf &, unsigned, unsigned, float) except +
    float get(const MatrixXf &, unsigned, unsigned) except +
