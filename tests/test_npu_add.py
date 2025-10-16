import numpy as np
from pyascend import GMem


def test_npu_add():
    size = 256
    x = np.random.rand(size).astype(np.float16)
    y = np.random.rand(size).astype(np.float16)
    assert np.allclose(GMem(x) + GMem(y), x + y, atol=1e-3)
