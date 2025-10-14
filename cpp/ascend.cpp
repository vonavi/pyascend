#include "ascend.hpp"

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

void writeFile(const std::string &filepath, char *data, size_t length) {
  std::ofstream ofs(filepath, std::ios::binary);
  if (!ofs)
    throw std::runtime_error("Failed to open " + filepath);
  if (!ofs.write(data, length))
    throw std::runtime_error("Failed to write " + filepath);
}

// ---- Main functions ----

void kernelLaunch(const std::string &kernel, const std::string &objPath,
                  const std::string &dataDir) {
  constexpr unsigned dataSize = 256;
  const size_t byteLenX = dataSize * sizeof(float16_t);
  const size_t byteLenY = dataSize * sizeof(float16_t);
  const size_t byteLenZ = dataSize * sizeof(float16_t);

  CHECK_ACL(aclInit(nullptr));
  const int deviceId = 0;
  CHECK_ACL(aclrtSetDevice(deviceId));

  char *binData = new char[MAX_BIN_LENGTH];
  size_t byteLen;
  readFile(objPath, binData, byteLen);
  rtDevBinary_t binary{.magic = RT_DEV_BINARY_MAGIC_ELF_AIVEC,
                       .version = 0,
                       .data = binData,
                       .length = byteLen};

  void *binHandle = nullptr;
  CHECK_RT(rtDevBinaryRegister(&binary, &binHandle));
  CHECK_RT(rtFunctionRegister(binHandle, kernel.c_str(), kernel.c_str(),
                              kernel.c_str(), FUNC_MODE_NORMAL));

  rtStream_t stream;
  CHECK_RT(rtStreamCreate(&stream, 0));

  // --- Input X ---
  void *hostX = nullptr;
  CHECK_ACL(aclrtMallocHost(&hostX, byteLenX));
  readFile(dataDir + "/input_x.bin", reinterpret_cast<char *>(hostX), byteLen);
  if (byteLen != byteLenX)
    throw std::runtime_error("Unexpected input X length " +
                             std::to_string(byteLen));

  void *deviceX = nullptr;
  CHECK_ACL(aclrtMalloc(&deviceX, byteLenX, ACL_MEM_MALLOC_HUGE_FIRST));
  CHECK_ACL(aclrtMemcpy(deviceX, byteLenX, hostX, byteLenX,
                        ACL_MEMCPY_HOST_TO_DEVICE));
  CHECK_ACL(aclrtFreeHost(hostX));

  // --- Input Y ---
  void *hostY = nullptr;
  CHECK_ACL(aclrtMallocHost(&hostY, byteLenY));
  readFile(dataDir + "/input_y.bin", reinterpret_cast<char *>(hostY), byteLen);
  if (byteLen != byteLenY)
    throw std::runtime_error("Unexpected input Y length " +
                             std::to_string(byteLen));

  void *deviceY = nullptr;
  CHECK_ACL(aclrtMalloc(&deviceY, byteLenY, ACL_MEM_MALLOC_HUGE_FIRST));
  CHECK_ACL(aclrtMemcpy(deviceY, byteLenY, hostY, byteLenY,
                        ACL_MEMCPY_HOST_TO_DEVICE));
  CHECK_ACL(aclrtFreeHost(hostY));

  // --- Output Z ---
  void *deviceZ = nullptr;
  CHECK_ACL(aclrtMalloc(&deviceZ, byteLenZ, ACL_MEM_MALLOC_HUGE_FIRST));

  struct Args {
    void *inX;
    void *inY;
    void *outZ;
    size_t size;
  } args{deviceX, deviceY, deviceZ, dataSize};
  CHECK_RT(rtKernelLaunch(kernel.c_str(), /*blockDim=*/1, &args, sizeof(args),
                          nullptr, stream));
  CHECK_RT(rtStreamSynchronize(stream));

  void *hostZ = nullptr;
  CHECK_ACL(aclrtMallocHost(&hostZ, byteLenZ));
  CHECK_ACL(aclrtMemcpy(hostZ, byteLenZ, deviceZ, byteLenZ,
                        ACL_MEMCPY_DEVICE_TO_HOST));
  writeFile(dataDir + "/output_z.bin", reinterpret_cast<char *>(hostZ),
            byteLenZ);

  CHECK_ACL(aclrtFree(deviceX));
  CHECK_ACL(aclrtFree(deviceY));
  CHECK_ACL(aclrtFree(deviceZ));
  CHECK_ACL(aclrtFreeHost(hostZ));

  CHECK_RT(rtStreamDestroy(stream));
  CHECK_RT(rtDevBinaryUnRegister(binHandle));
  delete[] binData;
  CHECK_ACL(aclrtResetDevice(deviceId));
  CHECK_ACL(aclFinalize());
}
