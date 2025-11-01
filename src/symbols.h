#ifndef SYMBOLS_H
#define SYMBOLS_H
#include <string>
#include <unordered_map>
#include <cstdint>

struct SymbolTable {
    std::unordered_map<std::string, uint32_t> labelToAddr;
    void add(const std::string &label, uint32_t addr);
    bool has(const std::string &label) const;
    uint32_t get(const std::string &label) const;
};

#endif
