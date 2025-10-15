#ifndef ASCEND_HPP
#define ASCEND_HPP

#include <cstddef> // std::byte
#include <string>
#include <vector>

typedef int16_t float16_t;

#define MAX_BIN_LENGTH 0x1000000

// ---- Error checking helpers ----

#define CHECK_ACL(x)                                                           \
  do {                                                                         \
    aclError __ret = x;                                                        \
    if (ACL_ERROR_NONE != __ret) {                                             \
      std::ostringstream oss;                                                  \
      oss << __FILE__ << ":" << __LINE__ << " aclError:" << __ret;             \
      throw std::runtime_error(oss.str());                                     \
    }                                                                          \
  } while (0)

#define CHECK_RT(x)                                                            \
  do {                                                                         \
    rtError_t __ret = x;                                                       \
    if (RT_ERROR_NONE != __ret) {                                              \
      std::ostringstream oss;                                                  \
      oss << __FILE__ << ":" << __LINE__ << " rtError:" << __ret;              \
      throw std::runtime_error(oss.str());                                     \
    }                                                                          \
  } while (0)

// ---- Main functions ----

void ascendInitialize();

void kernelLaunch(const std::string &kernel,
                  const std::vector<std::byte> &vectorX,
                  const std::vector<std::byte> &vectorY,
                  std::vector<std::byte> &vectorZ, const std::string &objPath);

#endif // ASCEND_HPP
