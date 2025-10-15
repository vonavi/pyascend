#include "ascend.hpp"

#include <cstring> // std::memcpy
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "acl/acl.h"
#include "acl/acl_rt.h"
#include "runtime/kernel.h"

// ---- File utilities ----

void readFile(const std::string &filepath, char *data, size_t &length) {
  std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
  if (!ifs)
    throw std::runtime_error("Failed to open " + filepath);

  length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  if (!ifs.read(data, length))
    throw std::runtime_error("Failed to read " + filepath);
}

// ---- GMem class ----

void GMem::copyFrom(const void *data, size_t nbytes) {
  void *host = nullptr;
  CHECK_ACL(aclrtMallocHost(&host, nbytes));
  std::memcpy(host, data, nbytes);

  CHECK_ACL(aclrtMalloc(&m_data, nbytes, ACL_MEM_MALLOC_HUGE_FIRST));
  CHECK_ACL(
      aclrtMemcpy(m_data, nbytes, host, nbytes, ACL_MEMCPY_HOST_TO_DEVICE));
  CHECK_ACL(aclrtFreeHost(host));
}

// ---- Main functions ----

void ascendInitialize() {
  CHECK_ACL(aclInit(nullptr));
  const int deviceId = 0;
  CHECK_ACL(aclrtSetDevice(deviceId));
}

void kernelLaunch(const std::string &kernel, const GMem &gmX, const GMem &gmY,
                  std::vector<std::byte> &vectorZ, const std::string &objPath) {
  char *binData = new char[MAX_BIN_LENGTH];
  size_t binLen;
  readFile(objPath, binData, binLen);
  rtDevBinary_t binary{.magic = RT_DEV_BINARY_MAGIC_ELF_AIVEC,
                       .version = 0,
                       .data = binData,
                       .length = binLen};

  void *binHandle = nullptr;
  CHECK_RT(rtDevBinaryRegister(&binary, &binHandle));
  CHECK_RT(rtFunctionRegister(binHandle, kernel.c_str(), kernel.c_str(),
                              kernel.c_str(), FUNC_MODE_NORMAL));

  rtStream_t stream;
  CHECK_RT(rtStreamCreate(&stream, 0));

  void *deviceZ = nullptr;
  size_t byteLenZ = vectorZ.size();
  CHECK_ACL(aclrtMalloc(&deviceZ, byteLenZ, ACL_MEM_MALLOC_HUGE_FIRST));

  size_t dataSize = byteLenZ / sizeof(float16_t);
  struct Args {
    void *inX;
    void *inY;
    void *outZ;
    size_t size;
  } args{gmX.data(), gmY.data(), deviceZ, dataSize};
  CHECK_RT(rtKernelLaunch(kernel.c_str(), /*blockDim=*/1, &args, sizeof(args),
                          nullptr, stream));
  CHECK_RT(rtStreamSynchronize(stream));

  void *hostZ = nullptr;
  CHECK_ACL(aclrtMallocHost(&hostZ, byteLenZ));
  CHECK_ACL(aclrtMemcpy(hostZ, byteLenZ, deviceZ, byteLenZ,
                        ACL_MEMCPY_DEVICE_TO_HOST));
  std::memcpy(vectorZ.data(), hostZ, byteLenZ);

  CHECK_ACL(aclrtFree(deviceZ));
  CHECK_ACL(aclrtFreeHost(hostZ));

  CHECK_RT(rtStreamDestroy(stream));
  CHECK_RT(rtDevBinaryUnRegister(binHandle));
  delete[] binData;
}
