#include "instruction.h"
#include <sstream>
#include <vector>

vector<string> split(const string &line) {
    stringstream ss(line);
    string word;
    vector<string> tokens;
    while (ss >> word)
        tokens.push_back(word);
    return tokens;
}

string assembleInstruction(const string &line) {
    auto tokens = split(line);
    if (tokens.empty()) return "";

    string op = tokens[0];
    if (op == "add")  return "0000000xxxxxxxxxx000xxxxx0110011";
    if (op == "sub")  return "0100000xxxxxxxxxx000xxxxx0110011";
    if (op == "and")  return "0000000xxxxxxxxxx111xxxxx0110011";
    if (op == "or")   return "0000000xxxxxxxxxx110xxxxx0110011";
    
    return "00000000000000000000000000000000"; // default
}
