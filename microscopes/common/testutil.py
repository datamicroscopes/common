"""routines to make writing test cases less painful

"""

import numpy as np
import itertools as it
from microscopes.common.util import (
    KL_approx,
    KL_discrete,
    logsumexp,
)
from nose.tools import (
    assert_equals,
    assert_almost_equals,
)


def assert_1d_lists_almost_equals(first,
                                  second,
                                  places=None,
                                  msg=None,
                                  delta=None):
    assert_equals(len(first), len(second), msg=msg)
    for i, j in zip(first, second):
        assert_almost_equals(i, j, places=places, msg=msg, delta=delta)


class OurAssertionError(Exception):

    def __init__(self, ex):
        self._ex = ex


def our_assert_almost_equals(first, second, places=None, msg=None, delta=None):
    try:
        assert_almost_equals(
            first, second, places=places, msg=msg, delta=delta)
    except AssertionError as ex:
        raise OurAssertionError(ex)


def assert_1d_cont_dist_approx_emp(sample_fn,
                                   log_q_fn,
                                   grid_min,
                                   grid_max,
                                   grid_n,
                                   ntries=5,
                                   nsamples=1000,
                                   kl_places=3,
                                   smoothing=1e-5):
    """
    Let p(x) be a valid continous scalar probability distribution, where q(x)
    \propto p(x) (p(x) is known up to the normalization constant)

    Assert that sample_fn produces samples from p(x) in the range of [grid_min,
    grid_max]

    This works by creating a grid of np.linspace(grid_min, grid_max, grid_n),
    evaluating log q(x) over the (midpoints of the) grid, and then creating
    a discrete distribution.  The same grid is then used to create a histogram
    from the outputs of sample_fn.

    The KL divergence of these two discrete probability distributions is then
    compared.
    """

    if grid_min >= grid_max:
        raise ValueError("invalid grid specified")
    if grid_n <= 1:
        raise ValueError("empty grid")
    if smoothing < 0.:
        raise ValueError("smoothing needs to be non-negative")

    # create the "true" distribution from log_q_fn
    grid = np.linspace(grid_min, grid_max, grid_n)
    points = (grid[1:] + grid[:-1]) / 2.

    # multiplicative smoothing (multiplying by ~1)
    true_dist = np.array([log_q_fn(pt) for pt in points]) + smoothing
    true_dist = scores_to_probs(true_dist)

    def discrete_sample_fn():
        sample = sample_fn()
        idx = np.searchsorted(grid, sample)
        if idx == 0:
            return 0
        if idx == grid_n:
            return idx - 2
        return idx - 1

    assert_discrete_dist_approx(discrete_sample_fn,
                                true_dist,
                                ntries,
                                nsamples,
                                kl_places)


def assert_1d_cont_dist_approx_sps(sample_fn,
                                   rv,
                                   support=None,
                                   ntries=5,
                                   nsamples=1000,
                                   nbins=1000,
                                   mean_places=3,
                                   var_places=3,
                                   kl_places=3):
    """
    Assert that the distributions of samples from sample_fn
    approaches the 1D continuous (real) distribution described by
    the (scipy.stats) rv object.

    Currently, three statistics are checked for convergence:
      (a) mean
      (b) variance
      (c) approximate KL-divergence
    """

    if support is None:
        support = rv.interval(1)
    if np.isinf(support[0]) or np.isinf(support[1]):
        raise ValueError("support is infinite: " + support)
    if support[1] <= support[0]:
        raise ValueError("support is empty")
    if ntries <= 0:
        raise ValueError("bad ntries: " + ntries)

    smoothing = 1e-5
    true_mean, true_var = rv.mean(), rv.var()
    raw_samples = []
    while 1:
        raw_samples.extend(sample_fn() for _ in xrange(nsamples))
        samples = np.array(raw_samples, dtype=np.float)
        try:
            # estimate mean
            est_mean = samples.mean()
            print 'true_mean', true_mean, 'est_mean', est_mean, \
                'diff', np.abs(true_mean - est_mean)
            our_assert_almost_equals(true_mean, est_mean, places=mean_places)

            # estimate variance
            est_var = samples.var(ddof=1)  # used unbiased estimator
            print 'true_var', true_var, 'est_var', est_var, \
                'diff', np.abs(true_var - est_var)
            our_assert_almost_equals(true_var, est_var, places=var_places)

            # estimate empirical KL
            bins = np.linspace(support[0], support[1], nbins)

            est_hist, _ = np.histogram(samples, bins=bins, density=False)
            est_hist = np.array(est_hist, dtype=np.float)
            est_hist += smoothing
            est_hist /= est_hist.sum()

            points = (bins[1:] + bins[:-1]) / 2.
            actual_hist = rv.pdf(points)
            actual_hist /= actual_hist.sum()

            kldiv = KL_approx(actual_hist, est_hist, bins[1] - bins[0])
            print 'kldiv:', kldiv
            our_assert_almost_equals(kldiv, 0., places=kl_places)

            return  # success
        except OurAssertionError as ex:
            print 'warning:', ex._ex.message
            ntries -= 1
            if not ntries:
                raise ex._ex


def assert_discrete_dist_approx(sample_fn,
                                dist,
                                ntries=5,
                                nsamples=1000,
                                kl_places=3):
    """
    Assert that the distributions of samples from sample_fn
    approaches the discrete distribution given by dist

    Currently, this is done by checking the KL-divergence
    (in both directions)
    """

    assert_almost_equals(dist.sum(), 1.0, places=4)

    if ntries <= 0:
        raise ValueError("bad ntries: " + ntries)

    smoothing = 1e-5
    est_hist = np.zeros(len(dist), dtype=np.int)
    while 1:
        for _ in xrange(nsamples):
            est_hist[sample_fn()] += 1
        try:
            hist = np.array(est_hist, dtype=np.float)
            hist += smoothing
            hist /= hist.sum()

            ab = KL_discrete(hist, dist)
            ba = KL_discrete(dist, hist)

            print 'KL_discrete(emp, act):', ab
            print 'KL_discrete(act, emp):', ba
            print 'act:', dist
            print 'emp:', hist

            our_assert_almost_equals(ab, 0., places=kl_places)
            our_assert_almost_equals(ba, 0., places=kl_places)

            return  # success
        except OurAssertionError as ex:
            print 'warning:', ex._ex.message
            ntries -= 1
            if not ntries:
                raise ex._ex


def permutation_canonical(assignments):
    assignments = np.copy(assignments)
    lowest = 0
    for i in xrange(assignments.shape[0]):
        if assignments[i] < lowest:
            continue
        if assignments[i] == lowest:
            lowest += 1
            continue
        temp = assignments[i]
        idxs = assignments == temp
        assignments[assignments == lowest] = temp
        assignments[idxs] = lowest
        lowest += 1
    return assignments


def permutation_iter(n):
    seen = set()
    for C in it.product(range(n), repeat=n):
        C = tuple(permutation_canonical(np.array(C)))
        if C in seen:
            continue
        seen.add(C)
        yield C


# XXX(stephentu): use the one in distributions
def scores_to_probs(scores):
    scores = np.array(scores)
    scores -= logsumexp(scores)
    scores = np.exp(scores)
    return scores


def dist_on_all_clusterings(score_fn, N):
    """
    Enumerate all possible clusterings of N entities, calling
    score_fn with each assignment.

    The reslting enumeration is then turned into a valid
    discrete probability distribution
    """
    return scores_to_probs(np.array(map(score_fn, permutation_iter(N))))
