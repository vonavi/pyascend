import numpy as np
from pyascend import GMem, kernel_launch


def test_add_custom():
    size = 256
    x = np.random.rand(size).astype(np.float16)
    y = np.random.rand(size).astype(np.float16)
    z = kernel_launch("add_custom", GMem(x), GMem(y))
    assert np.allclose(z, x + y, atol=1e-3)
