from microscopes.common.relation.dataview import \
    sparse_2d_dataview

import numpy as np
from scipy.sparse import coo_matrix, csr_matrix, csc_matrix

def test_build_from_csr():
    row  = np.array([0,3,1,0])
    col  = np.array([0,3,1,2])
    data = np.array([4,5,7,9])
    m = coo_matrix((data,(row,col)), shape=(4,4))
    csr = m.tocsr()
    assert isinstance(csr, csr_matrix)
    view = sparse_2d_dataview(csr)
    assert view.shape() == (4,4)

def test_build_from_csc():
    row  = np.array([0,3,1,0])
    col  = np.array([0,3,1,2])
    data = np.array([4,5,7,9])
    m = coo_matrix((data,(row,col)), shape=(4,4))
    csc = m.tocsc()
    assert isinstance(csc, csc_matrix)
    view = sparse_2d_dataview(csc)
    assert view.shape() == (4,4)
