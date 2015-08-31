from microscopes.models import (
    bb,
    bnb,
    gp,
    nich,
    dd,
    bbnc,
    niw,
    dm,
)

from nose.tools import assert_equals

import pickle


def test_model_callable():
    assert_equals(nich(), nich)


def test_models_dtype():
    assert_equals(niw(3).py_desc().get_np_dtype().shape, (3,))
    assert_equals(niw(5).py_desc().get_np_dtype().shape, (5,))


def test_models_pickle():
    models = (
        bb,
        bnb,
        gp,
        nich,
        dd(4),
        bbnc,
        niw(3),
        dm(5),
    )

    for model in models:
        bstr = pickle.dumps(model)
        model1 = pickle.loads(bstr)
        assert_equals(model.name(), model1.name())

        if model.name() == 'dd':
            assert_equals(len(model.default_hyperparams()['alphas']),
                          len(model1.default_hyperparams()['alphas']))
        elif model.name() == 'niw':
            assert_equals(len(model.default_hyperparams()['mu']),
                          len(model1.default_hyperparams()['mu']))
        elif model.name() == 'dm':
            assert_equals(model.py_desc().get_np_dtype().shape,
                          model1.py_desc().get_np_dtype().shape)
