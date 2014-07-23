# Dirichlet-Multinomial conjugate prior, using the same framework as
# distributions

import numpy as np
import math

from distributions.dbg.special import log, gammaln
from distributions.mixins import SharedMixin, GroupIoMixin, SharedIoMixin

NAME = 'DirichletMultinomial'
Value = int

class Shared(SharedMixin, SharedIoMixin):
    def __init__(self):
        self._alphas = None

    @property
    def dim(self):
        return len(self.alphas)

    def load(self, raw):
        self._alphas = np.array(raw['alphas'], dtype=np.float)

    def load_protobuf(self, message):
        self._alphas = np.array(message.alphas, dtype=np.float)

    def dump_protobuf(self, message):
        message.Clear()
        for alpha in self._alphas:
            message.alphas.append(alpha)

class Group(GroupIoMixin):
    def __init__(self):
        self._counts = None

    def init(self, shared):
        self._counts = np.zeros(shared.dim, dtype=np.int)

    def add_value(self, shared, value):
        for i, xi in value:
            self._counts[i] += xi

    def remove_value(self, shared, value):
        for i, xi in value:
            self._counts[i] -= xi

    def merge(self, shared, source):
        self._counts += source._counts

    def score_value(self, shared, value):
        x_sum = sum(value)
        a_sum = sum(shared._alphas)
        n_sum = sum(self._counts)
        score = 0.
        for xi, (ai, ni) in zip(value, zip(shared._alphas, self._counts)):
            score -= gammaln(xi + 1)
            score += xi * log(ai + ni)
        score += gammaln(x_sum + 1)
        score -= x_sum * log(a_sum + n_sum)
        return score

    def score_data(self, shared):
        a_sum = sum(shared._alphas)
        n_sum = sum(self._counts)
        score = 0.
        for ai, ni in zip(self._alphas, self._counts):
            score += gammaln(ni + ai) - gammaln(ai)
        score += gammaln(a_sum) - gammaln(a_sum + n_sum)
        return score

    def sample_value(self, shared):
        raise RuntimeError("Unimplemented")

    def load(self, raw):
        self._counts = np.array(raw['counts'], dtype=np.int)

    def dump(self):
        return {'counts': self.counts.tolist()}

    def load_protobuf(self, message):
        self._counts = np.array(message._counts, dtype=np.int)

    def dump_protobuf(self, message):
        message.Clear()
        for count in self._counts:
            message.counts.append(count)
