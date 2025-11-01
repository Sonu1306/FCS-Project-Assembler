#include "utils.h"

int regToNum(const string &reg) {
    if (reg[0] == 'x') return stoi(reg.substr(1));
    return -1;
}
