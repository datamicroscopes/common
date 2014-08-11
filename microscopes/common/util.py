"""
util.py - common utility functions
"""

import numpy as np
import os


def almost_eq(a, b, tol=1e-5):
    return (np.fabs(a - b) <= tol).all()


def rank(A):
    _, S, _ = np.linalg.svd(A)
    return int(np.sum(S >= 1e-10))


def random_orthogonal_matrix(m, n):
    A, _ = np.linalg.qr(np.random.random((m, n)))
    assert rank(A) == min(m, n)
    return A


def random_orthonormal_matrix(n):
    A = random_orthogonal_matrix(n, n)
    assert almost_eq(np.dot(A.T, A), np.eye(n))
    return A


def KL_discrete(a, b):
    return np.sum([p * np.log(p / q) for p, q in zip(a, b)])


def KL_approx(a, b, dA):
    return np.sum([p * np.log(p / q) * dA for p, q in zip(a, b)])

# This allows us to avoid having a dependency on scipy>=0.10.x,
# which speeds up travis builds
try:
    from scipy.misc import logsumexp
except ImportError:
    from numpy import exp, log, asarray, rollaxis, sum

    # Taken from:
    # https://github.com/scipy/scipy/blob/v0.14.0/scipy/misc/common.py
    def logsumexp(a, axis=None, b=None):
        """Compute the log of the sum of exponentials of input elements.

        Parameters
        ----------
        a : array_like
            Input array.
        axis : int, optional
            Axis over which the sum is taken. By default `axis` is None,
            and all elements are summed.

            .. versionadded:: 0.11.0
        b : array-like, optional
            Scaling factor for exp(`a`) must be of the same shape as `a` or
            broadcastable to `a`.

            .. versionadded:: 0.12.0

        Returns
        -------
        res : ndarray
            The result, ``np.log(np.sum(np.exp(a)))`` calculated in a numerically
            more stable way. If `b` is given then ``np.log(np.sum(b*np.exp(a)))``
            is returned.

        See Also
        --------
        numpy.logaddexp, numpy.logaddexp2

        Notes
        -----
        Numpy has a logaddexp function which is very similar to `logsumexp`, but
        only handles two arguments. `logaddexp.reduce` is similar to this
        function, but may be less stable.

        Examples
        --------
        >>> from scipy.misc import logsumexp
        >>> a = np.arange(10)
        >>> np.log(np.sum(np.exp(a)))
        9.4586297444267107
        >>> logsumexp(a)
        9.4586297444267107

        With weights

        >>> a = np.arange(10)
        >>> b = np.arange(10, 0, -1)
        >>> logsumexp(a, b=b)
        9.9170178533034665
        >>> np.log(np.sum(b*np.exp(a)))
        9.9170178533034647
        """
        a = asarray(a)
        if axis is None:
            a = a.ravel()
        else:
            a = rollaxis(a, axis)
        a_max = a.max(axis=0)
        if b is not None:
            b = asarray(b)
            if axis is None:
                b = b.ravel()
            else:
                b = rollaxis(b, axis)
            out = log(sum(b * exp(a - a_max), axis=0))
        else:
            out = log(sum(exp(a - a_max), axis=0))
        out += a_max
        return out

assert logsumexp


def random_assignment_vector(n):
    # see include/microscopes/common/util.hpp
    ngroups = min(100, n) + 1
    return [np.random.randint(low=0,  high=ngroups) for _ in xrange(n)]


def mkdirp(path):
    try:
        os.makedirs(path)
    except OSError as ex:
        import errno
        if ex.errno != errno.EEXIST:
            raise
