#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "ascend.hpp"
#include <dlfcn.h>

namespace py = pybind11;
namespace fs = std::filesystem;

PYBIND11_MODULE(_ascend, m) {
  m.doc() = "Pybind11-powered runtime for launching kernels on Ascend NPU.";

  Dl_info dlInfo;
  dladdr((void *)&PyInit__ascend, &dlInfo);
  fs::path moduleDir = fs::path(dlInfo.dli_fname).parent_path();
  fs::path kernelsDir = fs::canonical(moduleDir / "kernels");

  ascendInitialize();

  py::class_<GMem>(m, "GMem", py::buffer_protocol())
      .def(py::init([](const py::buffer &buffer) {
        py::buffer_info info = buffer.request();
        if (info.format[0] != py::dtype("float16").char_())
          throw py::type_error("Input must have dtype float16");
        if (info.ndim != 1)
          throw py::value_error("Input must be 1-D vector");
        return new GMem(info.ptr, info.shape[0], info.itemsize);
      }))
      .def_buffer([](const GMem &gm) {
        std::string format(1, py::dtype("float16").char_());
        size_t nbytes = gm.size() * gm.itemsize();
        char *data = new char[nbytes];
        gm.copyTo(data, nbytes);

        return py::buffer_info(
            data,           /* Pointer to buffer */
            gm.itemsize(),  /* Size of one scalar */
            format,         /* Python struct-style format descriptor */
            1,              /* Number of dimensions */
            {gm.size()},    /* Buffer dimensions */
            {gm.itemsize()} /* Strides (in bytes) for each index */
        );
      })
      .def(
          "__add__",
          [kernelsDir](const GMem &gmX, const GMem &gmY) {
            return addKernelLaunch(gmX, gmY, kernelsDir);
          },
          py::is_operator());
}
