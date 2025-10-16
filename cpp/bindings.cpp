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
        char *data = new char[gm.nbytes()];
        gm.copyTo(data);

        py::dtype dtype = py::dtype("float16");
        size_t size = gm.nbytes() / dtype.itemsize();
        std::string format(1, dtype.char_());
        return py::buffer_info(
            data,              /* Pointer to buffer */
            dtype.itemsize(),  /* Size of one scalar */
            format,            /* Python struct-style format descriptor */
            1,                 /* Number of dimensions */
            {size},            /* Buffer dimensions */
            {dtype.itemsize()} /* Strides (in bytes) for each index */
        );
      });

  m.def(
      "kernel_launch",
      [](const std::string &kernel, const GMem &gmX, const GMem &gmY,
         const std::string &objPath) {
        if (gmX.nbytes() != gmY.nbytes())
          throw py::value_error("Input vectors must have the same length");

        size_t itemsize = py::dtype("float16").itemsize();
        size_t size = gmX.nbytes() / itemsize;
        GMem gmZ(size, itemsize);

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
