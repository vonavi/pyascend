#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "ascend.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_ascend, m) {
  m.doc() = "Pybind11-powered runtime for launching kernels on Ascend NPU.";

  ascendInitialize();

  py::class_<GMem>(m, "GMem").def(py::init([](const py::buffer &buffer) {
    py::buffer_info info = buffer.request();
    if (info.format[0] != py::dtype("float16").char_())
      throw py::type_error("Input must have dtype float16");
    if (info.ndim != 1)
      throw py::value_error("Input must be 1-D vector");

    size_t nbytes = info.shape[0] * info.itemsize;
    return new GMem(info.ptr, nbytes);
  }));

  m.def(
      "kernel_launch",
      [](const std::string &kernel, const GMem &gmX, const GMem &gmY,
         const std::string &objPath) {
        if (gmX.nbytes() != gmY.nbytes())
          throw py::value_error("Input vectors must have the same length");

        size_t nbytes = gmX.nbytes();
        GMem gmZ(nbytes);
        kernelLaunch(kernel, gmX, gmY, gmZ, objPath);
        void *outDataZ = new char[nbytes];
        gmZ.copyTo(outDataZ);

        py::dtype dtype = py::dtype("float16");
        size_t size = nbytes / dtype.itemsize();
        return py::array(dtype, {size}, {dtype.itemsize()}, outDataZ);
      },
      py::arg("kernel"), py::arg("x"), py::arg("y"), py::pos_only(),
      py::arg("objpath"), "Launch a kernel on Ascend NPU.");
}
