#include "ascend.hpp"
#include <sstream>

std::string kernel_launch(const std::string &name) {
  std::ostringstream ss;
  ss << "Launching kernel: " << name << "\n";
  return ss.str();
}
