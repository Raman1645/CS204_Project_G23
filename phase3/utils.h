#ifndef UTILS_H
#define UTILS_H

#include <string>

// Utility functions
void loadMC(const std::string &filename);
std::string hex2bin(std::string hexStr);
std::string bin2hex(std::string binStr);

#endif // UTILS_H