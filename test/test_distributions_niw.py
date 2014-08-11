from microscopes.common.util import random_orthonormal_matrix, almost_eq
from microscopes.dbg.models import niw
from microscopes.dbg.models.niw import sample_iw as py_sample_iw
from microscopes.common.random import sample_inverse_wishart as cxx_sample_iw
from microscopes.common.rng import rng
from distributions.dbg.models import nich

import numpy as np
from nose.plugins.attrib import attr

#import rpy2.robjects as ro
#from rpy2.robjects.numpy2ri import activate
#activate()
#ro.r('library(MCMCpack)')


def _test_iw_sampler(sample_iw_fn):
    Q = random_orthonormal_matrix(2)
    nu = 4
    S = np.dot(Q, np.dot(np.diag([1.0, 0.5]), Q.T))
    invS = np.linalg.inv(S)
    #def r_sample_iw(nu, scale):
    #    return ro.r.riwish(nu, scale)
    #r_samples = [r_sample_iw(nu, S) for _ in xrange(10000)]

    true_mean = 1. / (nu - S.shape[0] - 1) * S

    ntries = 100
    py_samples = []
    while ntries:
        py_samples.extend([sample_iw_fn(nu, S) for _ in xrange(10000)])

        #r_mean = sum(r_samples) / len(r_samples)
        py_mean = sum(py_samples) / len(py_samples)

        print 'true:', true_mean
        #print 'r:', r_mean
        print 'py:', py_mean
        diff = np.linalg.norm(true_mean - py_mean)
        print 'F-norm:', diff
        if diff <= 0.1:
            return
        ntries -= 1

    assert False, "mean did not converge"


def test_iw_sampler_py():
    _test_iw_sampler(py_sample_iw)


def test_iw_sampler_cxx():
    r = rng()
    _test_iw_sampler(lambda nu, S: cxx_sample_iw(nu, S, r))


def test_niw_dist():
    # test by comparing to the 1D NIX model

    mu0 = np.array([30.0])
    lam0 = 0.3
    psi0 = np.array([[2.]])
    nu0 = 3

    # make the NIW case
    niw_shared = niw.Shared()
    niw_shared.load({'mu0': mu0, 'lambda': lam0, 'psi': psi0, 'nu': nu0})
    niw_group = niw.Group()
    niw_group.init(niw_shared)

    assert niw_shared.dimension() == 1

    # make the NIX case
    nix_shared = nich.Shared()
    nix_shared.load(
        {'mu': mu0[0], 'kappa': lam0, 'sigmasq': psi0[0, 0] / nu0, 'nu': nu0})
    nix_group = nich.Group()
    nix_group.init(nix_shared)

    data = np.array([4., 54., 3., -12., 7., 10.])
    for d in data:
        niw_group.add_value(niw_shared, np.array([d]))
        nix_group.add_value(nix_shared, d)

    # check marginals
    assert almost_eq(niw_group.score_data(niw_shared),
                     nix_group.score_data(nix_shared))

    # remove and check
    niw_group.remove_value(niw_shared, np.array([data[1]]))
    nix_group.remove_value(nix_shared, np.array([data[1]]))

    assert almost_eq(niw_group.score_data(niw_shared),
                     nix_group.score_data(nix_shared))

    # check posterior predictive
    values = np.array([32., -0.1])

    for value in values:
        assert almost_eq(niw_group.score_value(niw_shared, np.array([value])),
                         nix_group.score_value(nix_shared, value))


def test_niw_mv_dist():
    # XXX: not really a true test, we just run it with D > 1 to make
    # sure it doesn't crash

    Q = random_orthonormal_matrix(3)
    nu0 = 6
    psi0 = np.dot(Q, np.dot(np.diag([1.0, 0.5, 0.2]), Q.T))
    lam0 = 0.3

    niw_shared = niw.Shared()
    niw_shared.load(
        {'mu0': np.ones(3), 'lambda': lam0, 'psi': psi0, 'nu': nu0})
    niw_group = niw.Group()
    niw_group.init(niw_shared)

    niw_group.add_value(niw_shared, np.array([1., -3., 43.]))
    score0 = niw_group.score_data(niw_shared)
    ll0 = niw_group.score_value(niw_shared, np.ones(3))
    niw_group.add_value(niw_shared, np.array([34., 3., 2.]))
    niw_group.remove_value(niw_shared, np.array([34., 3., 2.]))
    score1 = niw_group.score_data(niw_shared)
    ll1 = niw_group.score_value(niw_shared, np.ones(3))

    assert almost_eq(score0, score1)
    assert almost_eq(ll0, ll1)
