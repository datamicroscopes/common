# a sanity test, see if we can import

from nose.tools import assert_almost_equals

def test_import_models():
    from microscopes.cxx.models import bb, bnb, gp, nich, dd, bbnc, niw
    assert bb and bnb and gp and nich and dd and bbnc and niw

def test_import_recarray_dataview():
    from microscopes.cxx.common.recarray.dataview import numpy_dataview
    assert numpy_dataview

def test_import_sparse_ndarray_dataview():
    from microscopes.cxx.common.sparse_ndarray.dataview import numpy_dataview
    assert numpy_dataview

def test_import_rng():
    from microscopes.cxx.common.rng import rng
    r = rng(12345)
    assert r

def test_log_exponential():
    from microscopes.cxx.common.scalar_functions import log_exponential
    import math
    lam = 2.
    fn = log_exponential(lam)
    x = 10.
    assert_almost_equals(math.log(lam*math.exp(-lam*x)), fn(x), places=5)
    assert math.isinf(fn(-10.))

def test_log_normal():
    from microscopes.cxx.common.scalar_functions import log_normal
    from scipy.stats import norm
    import math
    mu, sigma2 = 1.5, 3.2
    x = 6.3
    ours = log_normal(mu, sigma2)
    assert_almost_equals(ours(x), norm.logpdf(x, loc=mu, scale=math.sqrt(sigma2)))
