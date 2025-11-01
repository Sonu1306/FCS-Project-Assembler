#ifndef UTILS_H
#define UTILS_H
#include <string>
#include <cstdint>

int regToNum(const std::string &reg);          // "x5" -> 5
int32_t parseImmediate(const std::string &s); // decimal or 0x hex
std::string toHex32(uint32_t v);
std::string trim(const std::string &s);
std::string lower(const std::string &s);

#endif
