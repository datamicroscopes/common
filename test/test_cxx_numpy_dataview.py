from microscopes.cxx.common.dataview import numpy_dataview
from microscopes.cxx.common.rng import rng

import numpy as np
import numpy.ma as ma

def test_numpy_dataview():
    # XXX: numpy_dataview does not expose a python interface,
    # so the best we can do is construct it and make sure it
    # doesn't crash
    x = np.array([(False, 32.), (True, 943.), (False, -32.)], dtype=[('', bool), ('', float)])
    view = numpy_dataview(x)
    assert view
    assert view.size() == x.shape[0]
    r = rng(5432)
    view.view(True, r)
    view.view(False, r)

def test_numpy_dataview_masked():
    # XXX: likewise with above test
    x = ma.masked_array(
        np.array([(True, False, True, True, True)], dtype=[('', np.bool)]*5),
        mask=[(False, False, True, True, True)])
    view = numpy_dataview(x)
    assert view
    assert view.size() == x.shape[0]
