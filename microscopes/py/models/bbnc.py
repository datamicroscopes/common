# explicitly tracks the p-value as the BetaBernoulliNonConj class in:
#  https://github.com/ericmjonas/netmotifs/blob/master/irm/componentmodels.h
#
# but conforms to the same distribution interface as:
#   https://github.com/forcedotcom/distributions/blob/master/distributions/dbg/models/bb.py
#
# XXX: retain distributions' copyright notice here?

import numpy as np
import scipy as sp
import scipy.stats
import math

from distributions.dbg.special import log, gammaln
from distributions.dbg.random import sample_bernoulli, sample_beta
from distributions.mixins import SharedMixin, GroupIoMixin, SharedIoMixin

NAME = 'BetaBernoulliNonConj'
EXAMPLES = [
    {
        'shared': {'alpha': 0.5, 'beta': 2.0},
        'values': [False, False, True, False, True, True, False, False],
    },
]
Value = bool


class Shared(SharedMixin, SharedIoMixin):
    def __init__(self):
        self.alpha = None
        self.beta = None

    def load(self, raw):
        self.alpha = float(raw['alpha'])
        self.beta = float(raw['beta'])

    def dump(self):
        return {
            'alpha': self.alpha,
            'beta': self.beta,
        }

    def load_protobuf(self, message):
        self.alpha = float(message.alpha)
        self.beta = float(message.beta)

    def dump_protobuf(self, message):
        message.alpha = self.alpha
        message.beta = self.beta


class Group(GroupIoMixin):
    def __init__(self):
        self.heads = None
        self.tails = None
        self.p = None

    def init(self, shared):
        self.heads = 0
        self.tails = 0
        self.p = sample_beta(shared.alpha, shared.beta)

    def add_value(self, shared, value):
        if value:
            self.heads += 1
        else:
            self.tails += 1

    def remove_value(self, shared, value):
        if value:
            self.heads -= 1
        else:
            self.tails -= 1

    def merge(self, shared, source):
        #self.heads += source.heads
        #self.tails += source.tails

        # does merging these groups make sense?
        raise Exception("XXX: merging two groups with explicit p values")

    def score_value(self, shared, value):
        """samples a value using the explicit p"""
        return log(self.p) if value else log(1.-self.p)

    def score_data(self, shared):
        """computes the joint p(q, Y)"""
        prior = sp.stats.beta.logpdf(self.p, shared.alpha, shared.beta)
        if self.p >= 0. and self.p <= 1.:
            likelihood = self.heads * log(self.p) + self.tails * log(1.-self.p)
        else:
            likelihood = -np.inf
        return prior + likelihood

    def sample_value(self, shared):
        sampler = Sampler()
        sampler.init(shared, self)
        return sampler.eval(shared)

    ### XXX: not sure whether or not to include
    ### the H/T counts in the loading/unloading!

    def load(self, raw):
        #self.heads = raw['heads']
        #self.tails = raw['tails']
        self.p = raw['p']
        #assert self.p >= 0. and self.p <= 1.

    def dump(self):
        return {
            #'heads': self.heads,
            #'tails': self.tails,
            'p': self.p,
        }

    def load_protobuf(self, message):
        #self.heads = message.heads
        #self.tails = message.tails
        self.p = message.p

    def dump_protobuf(self, message):
        #message.heads = self.heads
        #message.tails = self.tails
        message.p = self.p


class Sampler(object):
    def init(self, shared, group=None):
        if group is None:
            self.p = sample_beta(shared.alpha, shared.beta)
        else:
            self.p = group.p

    def eval(self, shared):
        return sample_bernoulli(self.p)

def sample_group(shared, size):
    group = Group()
    group.init(shared)
    sampler = Sampler()
    sampler.init(shared, group)
    return [sampler.eval(shared) for _ in xrange(size)]
