#ifndef DIRECTIVES_H
#define DIRECTIVES_H
#include <string>
#include <vector>
#include <cstdint>

struct DataEntry { uint32_t addr; std::vector<uint8_t> bytes; std::string comment; };

void processDirectivePass1(const std::vector<std::string> &tokens, uint32_t &dataPtr);
void processDirectivePass2(const std::vector<std::string> &tokens, uint32_t &dataPtr, uint32_t baseDataAddr, std::vector<DataEntry> &out);

#endif
