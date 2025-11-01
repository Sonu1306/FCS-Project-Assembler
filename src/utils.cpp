#include "utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstdlib>

using namespace std;

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string lower(const string &s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), [](unsigned char c){ return tolower(c); });
    return r;
}

int regToNum(const string &reg) {
    string r = reg;
    // remove commas and whitespace
    r.erase(remove_if(r.begin(), r.end(), [](char c){ return c==',' || isspace((unsigned char)c); }), r.end());
    if (r.empty()) return 0;
    if (r[0]=='x' || r[0]=='X' || r[0]=='r' || r[0]=='R') r = r.substr(1);
    try { return stoi(r); } catch(...) { return 0; }
}

int32_t parseImmediate(const string &s) {
    string t = s;
    // remove commas
    t.erase(remove(t.begin(), t.end(), ','), t.end());
    // allow hex 0x...
    if (t.size() > 2 && t[0]=='0' && (t[1]=='x' || t[1]=='X')) {
        return (int32_t)strtol(t.c_str(), nullptr, 16);
    }
    try { return stoi(t); } catch(...) { return 0; }
}

string toHex32(uint32_t v) {
    char buf[12];
    sprintf(buf, "0x%08X", v);
    return string(buf);
}
