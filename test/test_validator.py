from microscopes.common import validator as V

from nose.tools import assert_raises


def test_validate_kwargs():
    kwargs = {'a': 1, 'b': 2}
    V.validate_kwargs(kwargs, ('a', 'b', 'c',))
    assert_raises(ValueError, V.validate_kwargs, kwargs, ('a',))


def test_validate_type():
    obj = "abc"
    V.validate_type(obj, str)
    assert_raises(ValueError, V.validate_type, obj, dict)


def test_validate_dict_like():
    obj = {}
    V.validate_dict_like(obj)
    obj = set()
    assert_raises(ValueError, V.validate_dict_like, obj)


def test_validate_len():
    obj = [1]
    V.validate_len(obj, 1)
    assert_raises(ValueError, V.validate_len, obj, 2)


# XXX(stephentu): more!
