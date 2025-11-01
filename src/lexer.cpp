#include "lexer.h"
#include "utils.h"
#include <sstream>
#include <algorithm>

using namespace std;

string stripComment(const string &line) {
    // support '#' and '//' style
    size_t p = line.find('#');
    if (p != string::npos) return line.substr(0,p);
    p = line.find("//");
    if (p != string::npos) return line.substr(0,p);
    return line;
}

bool isLabel(const string &token) {
    if (token.empty()) return false;
    return token.back() == ':';
}

vector<string> tokenize(const string &line) {
    string s = stripComment(line);
    s = trim(s);
    vector<string> out;
    if (s.empty()) return out;

    // split on spaces but keep operands like 0(x5) as single token
    string cur;
    for (size_t i=0;i<s.size();++i) {
        char c = s[i];
        if (isspace((unsigned char)c)) {
            if (!cur.empty()) { 
                // remove trailing commas
                while (!cur.empty() && cur.back()==',') cur.pop_back();
                out.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) {
        while (!cur.empty() && cur.back()==',') cur.pop_back();
        out.push_back(cur);
    }
    return out;
}
