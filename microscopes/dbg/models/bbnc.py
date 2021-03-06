# explicitly tracks the p-value as the BetaBernoulliNonConj class in:
#  https://github.com/ericmjonas/netmotifs/blob/master/irm/componentmodels.h
#
# but conforms to the same distribution interface as:
#   https://github.com/forcedotcom/distributions/blob/master/distributions/dbg/models/bb.py
#
# XXX: retain distributions' copyright notice here?

import numpy as np
import scipy as sp

from distributions.dbg.special import log
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

    def protobuf_load(self, message):
        self.alpha = float(message.alpha)
        self.beta = float(message.beta)

    def protobuf_dump(self, message):
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
        # does merging these groups make sense?
        raise Exception("XXX: merging two groups with explicit p values")

    def score_value(self, shared, value):
        """samples a value using the explicit p"""
        return log(self.p) if value else log(1. - self.p)

    def score_data(self, shared):
        """computes the joint p(q, Y)"""
        prior = sp.stats.beta.logpdf(self.p, shared.alpha, shared.beta)
        if self.p >= 0. and self.p <= 1.:
            likelihood = self.heads * \
                log(self.p) + self.tails * log(1. - self.p)
        else:
            likelihood = -np.inf
        return prior + likelihood

    def sample_value(self, shared):
        sampler = Sampler()
        sampler.init(shared, self)
        return sampler.eval(shared)

    def load(self, raw):
        self.p = raw['p']
        self.heads = raw['heads']
        self.tails = raw['tails']

    def dump(self):
        return {'p': self.p, 'heads': self.heads, 'tails': self.tails}

    def protobuf_load(self, message):
        self.p = message.p
        self.heads = message.heads
        self.tails = message.tails

    def protobuf_dump(self, message):
        message.p = self.p
        message.heads = self.heads
        message.tails = self.tails


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
