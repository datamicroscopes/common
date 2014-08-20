# common input argument validators go here

import collections


def _q(arg):
    return "`{}'".format(arg)


def validate_kwargs(kwargs, valid_kwargs):
    """
    validates kwargs.keys() against the whitelist valid_kwargs
    """
    valid_kwargs = set(valid_kwargs)
    for kwarg in kwargs.keys():
        if kwarg not in valid_kwargs:
            raise ValueError(
                "unexpected kwarg arg {} given. ".format(_q(kwarg)) +
                "valid kwargs are {}".format(", ".join(map(_q, valid_kwargs))))


def validate_type(obj, typ, param_name=None):
    if not isinstance(obj, typ):
        msg = "expected type {} for parameter{}, but got instance of type {}"
        raise ValueError(msg.format(
            repr(typ),
            "" if param_name is None else (" " + _q(param_name)),
            repr(type(obj))))


def validate_dict_like(obj, param_name=None):
    if (isinstance(obj, dict) or
            isinstance(obj, collections.Mapping)):
        return

    if (hasattr(obj, 'iteritems') and
            hasattr(obj, '__getitem__')):
        return

    msg = "expected dict-like for parameter{}, but got instance of type {}"
    raise ValueError(
        msg.format(
            "" if param_name is None else (" " + _q(param_name)),
            repr(type(obj))))


def validate_len(obj, size, param_name=None):
    if len(obj) != size:
        msg = "expected length {} for parameter{}, but got length {}"
        raise ValueError(msg.format(
            size,
            "" if param_name is None else (" " + _q(param_name)),
            len(obj)))


def validate_nonempty(lst, param_name=None):
    if not len(lst):
        msg = "expected non-empty value for parameter{}"
        raise ValueError(msg.format(
            "" if param_name is None else (" " + _q(param_name))))


def validate_positive(arg, param_name=None):
    if arg <= 0:
        msg = "expected positive value for parameter{}, got {}"
        raise ValueError(msg.format(
            "" if param_name is None else (" " + _q(param_name)),
            arg))


def validate_nonnegative(arg, param_name=None):
    if arg < 0:
        msg = "expected nonnegative value for parameter{}, got {}"
        raise ValueError(msg.format(
            "" if param_name is None else (" " + _q(param_name)),
            arg))


def validate_not_none(obj, param_name=None):
    if obj is None:
        msg = "NoneType given for parameter{}"
        raise ValueError(
            msg.format("" if param_name is None else (" " + _q(param_name))))


def validate_in_range(idx, n, param_name=None):
    if idx < 0 or idx >= n:
        msg = ("value {} is out of range for parameter{} "
               "(expected 0 <= idx < {})")
        raise ValueError(msg.format(
            idx,
            "" if param_name is None else (" " + _q(param_name)),
            n))
