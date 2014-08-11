# Dirichlet-Multinomial conjugate prior, using the same framework as
# distributions

import numpy as np

from distributions.dbg.special import gammaln
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

    def protobuf_load(self, message):
        self._alphas = np.array(message.alphas, dtype=np.float)

    def protobuf_dump(self, message):
        message.Clear()
        for alpha in self._alphas:
            message.alphas.append(alpha)


class Group(GroupIoMixin):

    def init(self, shared):
        self._counts = np.zeros(shared.dim, dtype=np.int)
        self._ratio = 0.

    def add_value(self, shared, value):
        count_sum = 0
        for i, xi in value:
            count_sum += xi
            self._counts[i] += xi
            self._ratio -= gammaln(xi + 1)
        self._ratio += gammaln(count_sum + 1)

    def remove_value(self, shared, value):
        count_sum = 0
        for i, xi in value:
            count_sum += xi
            self._counts[i] -= xi
            self._ratio += gammaln(xi + 1)
        self._ratio -= gammaln(count_sum + 1)

    def merge(self, shared, source):
        self._counts += source._counts
        self._ratio += source._ratio

    def score_value(self, shared, value):
        #x_sum = sum(value)
        #a_sum = sum(shared._alphas)
        #n_sum = sum(self._counts)
        #score = 0.
        #for xi, (ai, ni) in zip(value, zip(shared._alphas, self._counts)):
        #    score -= gammaln(xi + 1)
        #    score += xi * log(ai + ni)
        #score += gammaln(x_sum + 1)
        #score -= x_sum * log(a_sum + n_sum)
        #return score
        raise RuntimeError("need to fix")

    def score_data(self, shared):
        a_sum = sum(shared._alphas)
        n_sum = sum(self._counts)
        score = self._ratio
        for ai, ni in zip(self._alphas, self._counts):
            score += gammaln(ni + ai) - gammaln(ai)
        score += gammaln(a_sum) - gammaln(a_sum + n_sum)
        return score

    def sample_value(self, shared):
        raise RuntimeError("Unimplemented")

    def load(self, raw):
        self._counts = np.array(raw['counts'], dtype=np.int)
        self._ratio = float(raw['ratio'])

    def dump(self):
        return {'counts': self._counts.tolist(), 'ratio': self._ratio}

    def protobuf_load(self, message):
        self._counts = np.array(message.counts, dtype=np.int)
        self._ratio = message.ratio

    def protobuf_dump(self, message):
        message.Clear()
        for count in self._counts:
            message.counts.append(count)
        message.ratio = self._ratio
