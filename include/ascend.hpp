#ifndef ASCEND_HPP
#define ASCEND_HPP

#include <string>

void kernelLaunch(const std::string &kernel, const std::string &objPath,
                  const std::string &dataDir);

#endif // ASCEND_HPP
