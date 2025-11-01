#include <iostream>
#include <fstream>
#include <string>
#include "instruction.h"
using namespace std;

int main() {
    ifstream fin("input.asm");
    ofstream fout("output.mc");

    if (!fin.is_open()) {
        cerr << "Error: could not open input.asm" << endl;
        return 1;
    }

    string line;
    while (getline(fin, line)) {
        string machineCode = assembleInstruction(line);
        fout << machineCode << endl;
    }

    fin.close();
    fout.close();
    cout << "Assembly to machine code conversion complete!" << endl;
    return 0;
}
