#include "directives.h"
#include "utils.h"
#include <sstream>
#include <cstring>

using namespace std;

void processDirectivePass1(const vector<string> &tokens, uint32_t &dataPtr) {
    if (tokens.empty()) return;
    string d = lower(tokens[0]);
    if (d == ".byte") {
        // tokens 1..n are values
        dataPtr += (tokens.size()-1);
    } else if (d == ".half") {
        dataPtr += 2 * (tokens.size()-1);
    } else if (d == ".word") {
        dataPtr += 4 * (tokens.size()-1);
    } else if (d == ".dword") {
        dataPtr += 8 * (tokens.size()-1);
    } else if (d == ".asciz") {
        // tokens[1] combined string contents — join all tokens after first with spaces and remove quotes
        string s;
        for (size_t i=1;i<tokens.size();++i) {
            if (i>1) s += " ";
            s += tokens[i];
        }
        // strip surrounding quotes if present
        if (!s.empty() && s.front()=='"') s.erase(0,1);
        if (!s.empty() && s.back()=='"') s.pop_back();
        dataPtr += (uint32_t)(s.size()+1);
    }
}

void processDirectivePass2(const vector<string> &tokens, uint32_t &dataPtr, uint32_t baseDataAddr, vector<DataEntry> &out) {
    if (tokens.empty()) return;
    string d = lower(tokens[0]);
    if (d == ".byte") {
        for (size_t i=1;i<tokens.size();++i) {
            int v = parseImmediate(tokens[i]);
            DataEntry e; e.addr = dataPtr + baseDataAddr; e.bytes = { (uint8_t)(v & 0xFF) }; e.comment = tokens[i];
            out.push_back(e);
            dataPtr += 1;
        }
    } else if (d == ".half") {
        for (size_t i=1;i<tokens.size();++i) {
            int32_t v = parseImmediate(tokens[i]);
            DataEntry e; e.addr = dataPtr + baseDataAddr;
            e.bytes = { (uint8_t)(v & 0xFF), (uint8_t)((v>>8)&0xFF) };
            e.comment = tokens[i];
            out.push_back(e);
            dataPtr += 2;
        }
    } else if (d == ".word") {
        for (size_t i=1;i<tokens.size();++i) {
            int32_t v = parseImmediate(tokens[i]);
            DataEntry e; e.addr = dataPtr + baseDataAddr;
            e.bytes = { (uint8_t)(v & 0xFF), (uint8_t)((v>>8)&0xFF), (uint8_t)((v>>16)&0xFF), (uint8_t)((v>>24)&0xFF) };
            e.comment = tokens[i];
            out.push_back(e);
            dataPtr += 4;
        }
    } else if (d == ".dword") {
        for (size_t i=1;i<tokens.size();++i) {
            long long v = (long long)parseImmediate(tokens[i]);
            DataEntry e; e.addr = dataPtr + baseDataAddr;
            for (int b=0;b<8;++b) e.bytes.push_back((uint8_t)((v>>(8*b)) & 0xFF));
            e.comment = tokens[i];
            out.push_back(e);
            dataPtr += 8;
        }
    } else if (d == ".asciz") {
        string s;
        for (size_t i=1;i<tokens.size();++i) {
            if (i>1) s += " ";
            s += tokens[i];
        }
        if (!s.empty() && s.front()=='"') s.erase(0,1);
        if (!s.empty() && s.back()=='"') s.pop_back();
        DataEntry e; e.addr = dataPtr + baseDataAddr;
        for (char c : s) e.bytes.push_back((uint8_t)c);
        e.bytes.push_back(0);
        e.comment = s;
        out.push_back(e);
        dataPtr += (uint32_t)(s.size()+1);
    }
}
