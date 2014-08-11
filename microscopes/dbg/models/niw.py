# Gaussian conjugate prior, using the same framework as distributions

import numpy as np
import math

from microscopes.common.vendor.stats import (
    sample_niw as _sample_niw,
    sample_invwishart as _sample_iw,
)
from scipy.special import multigammaln
from distributions.dbg.special import gammaln
from distributions.mixins import SharedMixin, GroupIoMixin, SharedIoMixin

NAME = 'NormalInverseWishart'
Value = float

def score_student_t(x, nu, mu, sigma):
    """
    Eq. 313
    """
    d = x.shape[0]
    term1 = gammaln(nu/2. + d/2.) - gammaln(nu/2.)
    sigmainv = np.linalg.inv(sigma)
    term2 = -0.5*np.log(np.linalg.det(sigma)) - d/2.*np.log(nu*math.pi)
    diff = x - mu
    term3 = -0.5*(nu+d)*np.log(1. + 1./nu*np.dot(diff, np.dot(sigmainv, diff)))
    return term1 + term2 + term3

def sample_iw(nu0, psi0):
    return _sample_iw(psi0, nu0)

def sample_niw(mu0, kappa0, psi0, nu0):
    # matt calls the scale matrix "lmbda", we call it "psi"
    return _sample_niw(mu0, psi0, kappa0, nu0)

class Shared(SharedMixin, SharedIoMixin):
    def __init__(self):
        self._mu0 = None
        self._lam0 = None
        self._psi0 = None
        self._nu0 = None
        self._D = None

    def dimension(self):
        return self._D

    def load(self, raw):
        self._mu0 = raw['mu0']
        self._lam0 = float(raw['lambda'])
        self._psi0 = raw['psi']
        assert self._psi0.shape[0] == self._psi0.shape[1]
        self._nu0 = float(raw['nu'])
        self._D = self._psi0.shape[0]

    def dump(self):
        return {
            'mu0' : self._mu0,
            'lambda' : self._lam0,
            'psi' : self._psi0,
            'nu' : self._nu0,
        }

    def protobuf_load(self, message):
        self._mu0 = np.array(message.mu0, dtype=np.float)
        self._lam0 = getattr(message, 'lambda')
        self._D = self._mu0.shape[0]
        self._psi0 = np.array(message.psi, dtype=np.float)
        assert self._psi0.shape[0] == self._D * self._D
        self._psi0 = self._psi0.reshape((self._D, self._D))
        self._nu0 = message.nu
        assert self._nu0 > float(self._D) - 1.

    def protobuf_dump(self, message):
        message.Clear()
        for x in self._mu0:
            message.mu0.append(x)
        setattr(message, 'lambda', self._lam0)
        for x in self._psi0:
            for y in x:
                message.psi.append(y)
        message.nu = self._nu0

class Group(GroupIoMixin):
    def __init__(self):
        self._cnts = None
        self._sum_x = None
        self._sum_xxT = None

    def init(self, shared):
        D = shared._D
        self._cnts = 0
        self._sum_x = np.zeros(D)
        self._sum_xxT = np.zeros((D, D))

    def add_value(self, shared, value):
        self._cnts += 1
        self._sum_x += value
        self._sum_xxT += np.outer(value, value)

    def remove_value(self, shared, value):
        self._cnts -= 1
        self._sum_x -= value
        self._sum_xxT -= np.outer(value, value)

    def merge(self, shared, source):
        self._cnts += source._cnts
        self._sum_x += source._sum_x
        self._sum_xxT += source._sum_xxT

    def _post_params(self, shared):
        mu0, lam0, psi0, nu0 = shared._mu0, shared._lam0, shared._psi0, shared._nu0
        n, sum_x, sum_xxT = self._cnts, self._sum_x, self._sum_xxT
        xbar = sum_x / n if n else np.zeros(shared._D)
        mu_n = lam0/(lam0 + n)*mu0 + n/(lam0 + n)*xbar
        lam_n = lam0 + n
        nu_n = nu0 + n
        diff = xbar - mu0
        C_n = sum_xxT - np.outer(sum_x, xbar) - np.outer(xbar, sum_x) + n*np.outer(xbar, xbar)
        #print 'C_%d:'%(n), C_n
        psi_n = psi0 + C_n + lam0*n/(lam0+n)*np.outer(diff, diff)
        #print "postparams:"
        #print "  mu0" , mu_n
        #print "  lam" , lam_n
        #print "  psi" , psi_n
        #print "  nu"  , nu_n
        return mu_n, lam_n, psi_n, nu_n

    def score_value(self, shared, value):
        """
        Eq. 258
        """
        mu_n, lam_n, psi_n, nu_n = self._post_params(shared)
        D = shared.dimension()
        dof = nu_n-D+1.
        Sigma_n = psi_n*(lam_n+1.)/(lam_n*dof)
        #print 'DOF:', dof
        #print 'mean:', mu_n
        #print 'cov:', Sigma_n
        return score_student_t(value, dof, mu_n, Sigma_n)

    def score_data(self, shared):
        """
        Eq. 266
        """
        lam0, psi0, nu0 = shared._lam0, shared._psi0, shared._nu0
        mu_n, lam_n, psi_n, nu_n = self._post_params(shared)
        n = self._cnts
        D = shared.dimension()

        #T1 = multigammaln(nu_n/2., D)
        #T2 = + nu0/2.*np.log(np.linalg.det(psi0))
        #T3 = - (n*D/2.)*np.log(math.pi)
        #T4 = - multigammaln(nu0/2., D)
        #T5 = - nu_n/2.*np.log(np.linalg.det(psi_n))
        #T6 = + D/2.*np.log(lam0/lam_n)

        #print 'py T1', T1, D, nu_n/2.
        #print 'py T2', T2
        #print 'py T3', T3
        #print 'py T4', T4
        #print 'py T5', T5
        #print 'py T6', T6

        return multigammaln(nu_n/2., D) \
            + nu0/2.*np.log(np.linalg.det(psi0)) \
            - (n*D/2.)*np.log(math.pi) \
            - multigammaln(nu0/2., D) \
            - nu_n/2.*np.log(np.linalg.det(psi_n)) \
            + D/2.*np.log(lam0/lam_n)

    def sample_value(self, shared):
        sampler = Sampler()
        sampler.init(shared, self)
        return sampler.eval(shared)

    def load(self, raw):
        self._cnts = int(raw['count'])
        assert self._cnts >= 0
        self._sum_x = raw['sum_x']
        self._sum_xxT = raw['sum_xxT']
        D = self._sum_x.shape[0]
        assert self._sum_xxT.shape == (D,D)

    def dump(self):
        return {
            'count' : self._cnts,
            'sum_x' : self._sum_x,
            'sum_xxT' : self._sum_xxT,
        }

    def protobuf_load(self, message):
        self._cnts = message.count
        self._sum_x = np.array(message.sum_x, dtype=np.float)
        self._sum_xxT = np.array(message.sum_xxT, dtype=np.float)
        D = self._sum_x.shape[0]
        self._sum_xxT = self._sum_xxT.reshape((D, D))

    def protobuf_dump(self, message):
        message.Clear()
        message.count = self._cnts
        for x in self._sum_x:
            message.sum_x.append(x)
        for x in self._sum_xxT:
            for y in x:
                message.sum_xxT.append(y)

class Sampler(object):
    def init(self, shared, group=None):
        if group is not None:
            mu0, lam0, psi0, nu0 = group._post_params(self)
        else:
            mu0, lam0, psi0, nu0 = shared._mu0, shared._lam0, shared._psi0, shared._nu0
        self._mu, self._sigma = sample_niw(mu0, lam0, psi0, nu0)

    def eval(self, shared):
        return np.random.multivariate_normal(self._mu, self._sigma)

def sample_group(shared, size):
    group = Group()
    group.init(shared)
    sampler = Sampler()
    sampler.init(shared, group)
    return [sampler.eval(shared) for _ in xrange(size)]
