#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

#include "utils.h"
#include "symbols.h"
#include "lexer.h"
#include "encoder.h"
#include "directives.h"

using namespace std;

const uint32_t TEXT_BASE = 0x00000000u;
const uint32_t DATA_BASE = 0x10000000u;

int main(int argc, char** argv) {
    string inname = "input.asm";
    string outname = "output.mc";
    if (argc >= 2) inname = argv[1];
    ifstream fin(inname);
    if (!fin.is_open()) { cerr << "Error: cannot open " << inname << endl; return 1; }

    // PASS 1: build symbol table and compute sizes
    SymbolTable sym;
    uint32_t pc = TEXT_BASE;
    uint32_t dataPtr = 0; // offset from DATA_BASE
    bool inText = false, inData = false;
    vector<string> lines; // keep all lines for pass 2
    string line;
    while (getline(fin, line)) {
        lines.push_back(line);
        auto toks = tokenize(line);
        if (toks.empty()) continue;
        // label?
        if (isLabel(toks[0])) {
            string lbl = toks[0].substr(0, toks[0].size()-1);
            if (inText) sym.add(lbl, pc);
            else if (inData) sym.add(lbl, DATA_BASE + dataPtr);
            // remove label token
            toks.erase(toks.begin());
            if (toks.empty()) continue;
        }
        string op = lower(toks[0]);
        if (op == ".text") { inText = true; inData = false; continue; }
        if (op == ".data") { inData = true; inText = false; continue; }
        if (inText) {
            // assume each instruction is 4 bytes
            pc += 4;
        } else if (inData) {
            processDirectivePass1(toks, dataPtr);
        }
    }
    fin.close();

    // PASS 2: encode and write output
    fin.open(inname);
    ofstream fout(outname);
    if (!fout.is_open()) { cerr << "Error: cannot open output " << outname << endl; return 1; }

    pc = TEXT_BASE;
    dataPtr = 0;
    inText = false; inData = false;
    vector<pair<uint32_t, uint32_t>> textOut; // (addr, machine)
    vector<DataEntry> dataOut;

    for (size_t li=0; li<lines.size(); ++li) {
        string l = lines[li];
        auto toks = tokenize(l);
        if (toks.empty()) continue;
        if (isLabel(toks[0])) {
            toks.erase(toks.begin());
            if (toks.empty()) continue;
        }
        if (toks.empty()) continue;
        string op = lower(toks[0]);
        if (op == ".text") { inText=true; inData=false; continue; }
        if (op == ".data") { inData=true; inText=false; continue; }
        if (inText) {
            auto encoded = encodeInstruction(toks, pc, sym);
            uint32_t word = encoded.first;
            textOut.push_back({pc, word});
            // write line per spec: address hex, machinecode hex , asm # fields
            // format comment
            stringstream comment;
            comment << encoded.second;
            // print
            fout << "0x" << hex << pc << dec << " " << toHex32(word) << " , " << comment.str() << "\n";
            pc += 4;
        } else if (inData) {
            // directives
            processDirectivePass2(toks, dataPtr, DATA_BASE, dataOut);
        }
    }

    // After pass2 write data segment entries to output file
    for (auto &d : dataOut) {
        // print each DataEntry as hex of the whole chunk (if small)
        // for simplicity print bytes as hex sequence
        fout << "0x" << hex << d.addr << dec << " ";
        // create a hex representation of bytes in little-endian value if length <=8
        if (d.bytes.size() <= 8) {
            uint64_t v = 0;
            for (size_t i=0;i<d.bytes.size();++i) v |= ((uint64_t)d.bytes[i]) << (8*i);
            char buf[32]; sprintf(buf, "0x%0*llX", (int)(d.bytes.size()*2), (unsigned long long)v);
            fout << buf;
        } else {
            // for long strings, print ascii in comment
            fout << "\"";
            for (auto b : d.bytes) { if (b==0) fout << "\\0"; else fout << (char)b; }
            fout << "\"";
        }
        fout << " , " << d.comment << "\n";
    }

    fout.close();
    cout << "Assembly to machine code conversion complete!" << endl;
    return 0;
}
