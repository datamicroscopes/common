# a sanity test, see if we can import

def test_import_models():
    from microscopes.cxx.models import bb, bnb, gp, nich
    assert bb and bnb and gp and nich

def test_import_dataview():
    from microscopes.cxx.common.dataview import numpy_dataview
    assert numpy_dataview

def test_import_rng():
    from microscopes.cxx.common.rng import rng
    r = rng(12345)
    assert r
