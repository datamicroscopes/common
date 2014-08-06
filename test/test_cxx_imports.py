# a sanity test, see if we can import

from nose.tools import assert_almost_equals
import numpy as np

def test_import_models():
    from microscopes.models import \
        bb, bnb, gp, nich, dd, bbnc, niw, dm
    assert bb and bnb and gp and nich and dd and bbnc and niw and dm

def test_import_recarray_dataview():
    from microscopes.common.recarray.dataview import numpy_dataview
    assert numpy_dataview

def test_import_relation_dataview():
    from microscopes.common.relation.dataview import numpy_dataview
    from microscopes.common.relation.dataview import sparse_2d_dataview
    assert numpy_dataview and sparse_2d_dataview

def test_import_rng():
    from microscopes.common.rng import rng
    r = rng(12345)
    assert r

def test_log_exponential():
    from microscopes.common.scalar_functions import log_exponential
    import math
    lam = 2.
    fn = log_exponential(lam)
    x = 10.
    assert_almost_equals(math.log(lam*math.exp(-lam*x)), fn(x), places=5)
    assert math.isinf(fn(-10.))

def test_log_normal():
    from microscopes.common.scalar_functions import log_normal
    from scipy.stats import norm
    import math
    mu, sigma2 = 1.5, 3.2
    x = 6.3
    ours = log_normal(mu, sigma2)
    assert_almost_equals(ours(x), norm.logpdf(x, loc=mu, scale=math.sqrt(sigma2)))

def test_log_noninformative_beta_prior():
    from microscopes.common.scalar_functions import log_noninformative_beta_prior
    alpha, beta = 0.8, 0.2
    assert log_noninformative_beta_prior.input_dim() == 2
    val = log_noninformative_beta_prior(alpha, beta)
    assert_almost_equals(val, -2.5*np.log(alpha + beta), places=5)
