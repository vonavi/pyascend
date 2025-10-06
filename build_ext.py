import shutil
from pathlib import Path
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import Distribution


def build():
    ext_modules = [
        Pybind11Extension(
            "pyascend._ascend",
            sources=["cpp/ascend.cpp", "cpp/bindings.cpp"],
            include_dirs=["include"],
            cxx_std=17,
        )
    ]

    distribution = Distribution(
        {"name": "pyascend", "ext_modules": ext_modules}
    )

    cmd = build_ext(distribution)
    cmd.ensure_finalized()
    cmd.run()

    # Copy built extensions back to the project
    for output in cmd.get_outputs():
        output = Path(output)
        relative_extension = Path("src") / output.relative_to(cmd.build_lib)
        shutil.copyfile(output, relative_extension)


if __name__ == "__main__":
    build()
