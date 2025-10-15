import os
import numpy as np

from pyascend import kernel_launch

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def test_add_custom():
    size = 256
    x = np.random.rand(size).astype(np.float16)
    y = np.random.rand(size).astype(np.float16)
    z = kernel_launch(
        "add_custom", x, y, objpath=os.path.join(SCRIPT_DIR, "add_custom.o")
    )
    assert np.allclose(z, x + y, atol=1e-3)
