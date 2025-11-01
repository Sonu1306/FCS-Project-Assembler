#include "symbols.h"
#include <stdexcept>

using namespace std;

void SymbolTable::add(const string &label, uint32_t addr) {
    labelToAddr[label] = addr;
}

bool SymbolTable::has(const string &label) const {
    return labelToAddr.find(label) != labelToAddr.end();
}

uint32_t SymbolTable::get(const string &label) const {
    auto it = labelToAddr.find(label);
    if (it == labelToAddr.end()) throw runtime_error("Undefined label: " + label);
    return it->second;
}
