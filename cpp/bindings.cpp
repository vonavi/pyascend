#include "ascend.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(_ascend, m) {
  m.doc() = "Pybind11-powered runtime for launching kernels on Ascend NPU.";

  m.def("kernel_launch", &kernel_launch, py::arg("name"),
        "Launch a kernel by name.");
}
