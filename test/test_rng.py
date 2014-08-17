from microscopes.common.rng import rng

from nose.tools import assert_equals


def test_rng():
    r0 = rng(4382)
    r1 = rng(4382)

    for _ in xrange(100):
        assert_equals(r0.next(), r1.next())
