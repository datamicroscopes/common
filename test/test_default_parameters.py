from microscopes.models import bb, bnb, gp, nich, dd, bbnc, niw


def test_default_parameters():
    models = [bb, bnb, gp, nich, dd(5), bbnc, niw(10)]
    for m in models:
        typ = m.py_desc()._model_module
        s = typ.Shared()
        s.load(m.default_hyperparams())
