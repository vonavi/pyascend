#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "ascend.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_ascend, m) {
  m.doc() = "Pybind11-powered runtime for launching kernels on Ascend NPU.";

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
        char *data = new char[gm.size() * gm.itemsize()];
        gm.copyTo(data);

        return py::buffer_info(
            data,           /* Pointer to buffer */
            gm.itemsize(),  /* Size of one scalar */
            format,         /* Python struct-style format descriptor */
            1,              /* Number of dimensions */
            {gm.size()},    /* Buffer dimensions */
            {gm.itemsize()} /* Strides (in bytes) for each index */
        );
      });

  m.def(
      "kernel_launch",
      [](const std::string &kernel, const GMem &gmX, const GMem &gmY,
         const std::string &objPath) {
        if (gmX.itemsize() != gmY.itemsize())
          throw py::type_error("Inputs must have the same item size");
        if (gmX.size() != gmY.size())
          throw py::value_error("Inputs must have the same size");

        size_t size = gmX.size();
        GMem gmZ(size, gmX.itemsize());

        struct __attribute__((packed)) Args {
          void *inX __attribute__((aligned(8)));
          void *inY __attribute__((aligned(8)));
          void *outZ __attribute__((aligned(8)));
          size_t size __attribute__((aligned(4)));
        };
        Args args{gmX.data(), gmY.data(), gmZ.data(), size};
        std::byte *args_begin = reinterpret_cast<std::byte *>(&args);
        std::vector<std::byte> argBytes(args_begin, args_begin + sizeof(args));

        kernelLaunch(kernel, argBytes, objPath);
        return gmZ;
      },
      py::arg("kernel"), py::arg("x"), py::arg("y"), py::pos_only(),
      py::arg("objpath"), "Launch a kernel on Ascend NPU.");
}
