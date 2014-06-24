import numpy as np

class _numpy_iter(object):
    def __init__(self, Y, inds=None):
        self._Y = Y
        self._i = 0
        self._N = Y.shape[0]
        self._inds = inds

    def __iter__(self):
        return self

    def next(self):
        if self._i == self._N:
            raise StopIteration()
        i = self._inds[self._i] if self._inds is not None else self._i
        self._i += 1
        return i, self._Y[i]

class numpy_dataset(object):
    def __init__(self, Y):
        self._Y = Y

    def data(self, shuffle=False):
        inds = np.random.permutation(self.size()) if shuffle else None
        return _numpy_iter(self._Y, inds)

    def size(self):
        return self._Y.shape[0]
