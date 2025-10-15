#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "ascend.hpp"
#include <cstring> // std::memcpy

namespace py = pybind11;

PYBIND11_MODULE(_ascend, m) {
  m.doc() = "Pybind11-powered runtime for launching kernels on Ascend NPU.";

  ascendInitialize();

  m.def(
      "kernel_launch",
      [](const std::string &kernel, const py::array &inputX,
         const py::array &inputY, const std::string &objPath) {
        auto dtype = py::dtype("float16");

        if (!inputX.dtype().is(dtype) || !inputY.dtype().is(dtype))
          throw py::type_error("Inputs must have dtype float16");
        if (inputX.ndim() != 1 || inputY.ndim() != 1)
          throw py::value_error("Inputs must be 1-D vectors");
        if (inputX.size() != inputY.size())
          throw py::value_error("Input vectors must have the same length");

        size_t byteLen = inputX.nbytes();
        std::vector<std::byte> vectorX(byteLen);
        std::vector<std::byte> vectorY(byteLen);
        std::vector<std::byte> vectorZ(byteLen);

        std::memcpy(vectorX.data(), inputX.data(), byteLen);
        std::memcpy(vectorY.data(), inputY.data(), byteLen);
        kernelLaunch(kernel, vectorX, vectorY, vectorZ, objPath);

        return py::array(dtype, {inputX.size()}, {dtype.itemsize()},
                         vectorZ.data());
      },
      py::arg("kernel"), py::arg("x"), py::arg("y"), py::pos_only(),
      py::arg("objpath"), "Launch a kernel on Ascend NPU.");
}
