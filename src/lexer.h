#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <vector>

std::vector<std::string> tokenize(const std::string &line);
bool isLabel(const std::string &token); // "loop:" -> true
std::string stripComment(const std::string &line);

#endif
