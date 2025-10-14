#include "ascend.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(_ascend, m) {
  m.doc() = "Pybind11-powered runtime for launching kernels on Ascend NPU.";

  m.def("kernel_launch", &kernelLaunch, py::arg("kernel"), py::pos_only(),
        py::arg("objpath"), py::arg("datadir"),
        "Launch a kernel on Ascend NPU.");
}
