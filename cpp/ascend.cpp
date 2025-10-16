#include "ascend.hpp"

#include <cstring> // std::memcpy
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "acl/acl.h"
#include "acl/acl_rt.h"
#include "runtime/kernel.h"

namespace fs = std::filesystem;

// ---- File utilities ----

std::vector<char> readFile(const fs::path &filepath) {
  std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
  if (!ifs)
    throw std::runtime_error("Failed to open file: " + filepath.string());

  std::streamsize size = ifs.tellg();
  std::vector<char> buffer(size);

  ifs.seekg(0, std::ios::beg);
  if (!ifs.read(buffer.data(), size))
    throw std::runtime_error("Failed to read file: " + filepath.string());

  return buffer;
}

// ---- GMem class ----

GMem::GMem(size_t size, size_t itemsize) : m_size(size), m_itemsize(itemsize) {
  alloc(size * itemsize);
}

GMem::GMem(const void *data, size_t size, size_t itemsize)
    : m_size(size), m_itemsize(itemsize) {
  copyFrom(data, size * itemsize);
}

GMem::~GMem() { aclrtFree(m_data); }

void GMem::alloc(size_t nbytes) {
  CHECK_ACL(aclrtMalloc(&m_data, nbytes, ACL_MEM_MALLOC_HUGE_FIRST));
}

void GMem::copyFrom(const void *data, size_t nbytes) {
  void *host = nullptr;
  CHECK_ACL(aclrtMallocHost(&host, nbytes));
  std::memcpy(host, data, nbytes);

  CHECK_ACL(aclrtMalloc(&m_data, nbytes, ACL_MEM_MALLOC_HUGE_FIRST));
  CHECK_ACL(
      aclrtMemcpy(m_data, nbytes, host, nbytes, ACL_MEMCPY_HOST_TO_DEVICE));
  CHECK_ACL(aclrtFreeHost(host));
}

void GMem::copyTo(void *data, size_t nbytes) const {
  void *host = nullptr;
  CHECK_ACL(aclrtMallocHost(&host, nbytes));
  CHECK_ACL(
      aclrtMemcpy(host, nbytes, m_data, nbytes, ACL_MEMCPY_DEVICE_TO_HOST));

  std::memcpy(data, host, nbytes);
  CHECK_ACL(aclrtFreeHost(host));
}

// ---- Main functions ----

void ascendInitialize() {
  CHECK_ACL(aclInit(nullptr));
  const int deviceId = 0;
  CHECK_ACL(aclrtSetDevice(deviceId));
}

void kernelLaunch(const std::string &kernel, std::vector<char> &argBytes,
                  const fs::path &objectPath) {
  const std::vector<char> &binaryBuf = readFile(objectPath);
  rtDevBinary_t binary{.magic = RT_DEV_BINARY_MAGIC_ELF_AIVEC,
                       .version = 0,
                       .data = binaryBuf.data(),
                       .length = binaryBuf.size()};

  void *binaryHandle = nullptr;
  CHECK_RT(rtDevBinaryRegister(&binary, &binaryHandle));
  CHECK_RT(rtFunctionRegister(binaryHandle, kernel.c_str(), kernel.c_str(),
                              kernel.c_str(), FUNC_MODE_NORMAL));

  rtStream_t stream;
  CHECK_RT(rtStreamCreate(&stream, 0));
  CHECK_RT(rtKernelLaunch(kernel.c_str(), /*blockDim=*/1, argBytes.data(),
                          argBytes.size(), nullptr, stream));
  CHECK_RT(rtStreamSynchronize(stream));

  CHECK_RT(rtStreamDestroy(stream));
  CHECK_RT(rtDevBinaryUnRegister(binaryHandle));
}

GMem addKernelLaunch(const GMem &gmX, const GMem &gmY,
                     const fs::path &kernelsDir) {
  if (gmX.itemsize() != gmY.itemsize())
    throw std::invalid_argument("Inputs must have the same item size");
  if (gmX.size() != gmY.size())
    throw std::invalid_argument("Inputs must have the same size");

  struct __attribute__((packed)) Args {
    void *inX __attribute__((aligned(8)));
    void *inY __attribute__((aligned(8)));
    void *outZ __attribute__((aligned(8)));
    size_t size __attribute__((aligned(4)));
  };

  size_t size = gmX.size();
  GMem gmZ(size, gmX.itemsize());
  Args args{gmX.data(), gmY.data(), gmZ.data(), size};
  char *args_begin = reinterpret_cast<char *>(&args);
  std::vector<char> argBytes(args_begin, args_begin + sizeof(args));

  kernelLaunch("add", argBytes, kernelsDir / "add_kernel.o");
  return gmZ;
}
