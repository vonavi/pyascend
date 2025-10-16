#ifndef ASCEND_HPP
#define ASCEND_HPP

#include <cstddef> // std::byte
#include <filesystem>
#include <string>
#include <vector>

#include "acl/acl_rt.h"

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

// ---- GMem class ----

class GMem {
public:
  GMem(size_t size, size_t itemsize) : m_size(size), m_itemsize(itemsize) {
    alloc(size * itemsize);
  }
  GMem(const void *data, size_t size, size_t itemsize)
      : m_size(size), m_itemsize(itemsize) {
    copyFrom(data, size * itemsize);
  }
  ~GMem() {
    if (m_data)
      aclrtFree(m_data);
  }

  void *data() const { return m_data; }
  size_t size() const { return m_size; }
  size_t itemsize() const { return m_itemsize; }

  void alloc(size_t nbytes);
  void copyFrom(const void *data, size_t nbytes);
  void copyTo(void *data, size_t nbytes) const;

private:
  void *m_data = nullptr;
  size_t m_size = 0;
  size_t m_itemsize = 1;
};

// ---- Main functions ----

void ascendInitialize();

void kernelLaunch(const std::string &kernel, std::vector<std::byte> &argBytes,
                  const std::string &objPath);

GMem addKernelLaunch(const GMem &gmX, const GMem &gmY,
                     const std::filesystem::path &kernelsDir);

#endif // ASCEND_HPP
