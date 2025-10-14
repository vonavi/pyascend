import os
import numpy as np
from tempfile import TemporaryDirectory

from pyascend import kernel_launch

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SIZE = 256


def gen_inputs(temp_dir):
    input_x = np.random.rand(SIZE).astype(np.float16)
    input_y = np.random.rand(SIZE).astype(np.float16)

    input_x.tofile(os.path.join(temp_dir, "input_x.bin"))
    input_y.tofile(os.path.join(temp_dir, "input_y.bin"))


def check_output(temp_dir):
    x = np.fromfile(os.path.join(temp_dir, "input_x.bin"), dtype=np.float16)
    y = np.fromfile(os.path.join(temp_dir, "input_y.bin"), dtype=np.float16)
    z = np.fromfile(os.path.join(temp_dir, "output_z.bin"), dtype=np.float16)
    assert np.allclose(z, x + y, atol=1e-3)


def test_add_custom():
    with TemporaryDirectory() as temp_dir:
        gen_inputs(temp_dir)
        kernel_launch(
            "add_custom",
            objpath=os.path.join(SCRIPT_DIR, "add_custom.o"),
            datadir=temp_dir,
        )
        check_output(temp_dir)
