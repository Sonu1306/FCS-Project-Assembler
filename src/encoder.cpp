#include "encoder.h"
#include "utils.h"
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <sstream>
#include <iomanip>

using namespace std;

/* Basic RISC-V encoders (32-bit layout) */
uint32_t encodeR(uint8_t funct7, uint8_t rs2, uint8_t rs1, uint8_t funct3, uint8_t rd, uint8_t opcode) {
    return ((uint32_t)funct7 << 25) | ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) | ((uint32_t)funct3 << 12) | ((uint32_t)rd << 7) | opcode;
}
uint32_t encodeI(int32_t imm, uint8_t rs1, uint8_t funct3, uint8_t rd, uint8_t opcode) {
    uint32_t imm12 = (uint32_t)imm & 0xFFF;
    return (imm12 << 20) | ((uint32_t)rs1 << 15) | ((uint32_t)funct3 << 12) | ((uint32_t)rd << 7) | opcode;
}
uint32_t encodeS(int32_t imm, uint8_t rs2, uint8_t rs1, uint8_t funct3, uint8_t opcode) {
    uint32_t imm12 = (uint32_t)imm & 0xFFF;
    uint32_t imm11_5 = (imm12 >> 5) & 0x7F;
    uint32_t imm4_0  = imm12 & 0x1F;
    return (imm11_5 << 25) | ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) | ((uint32_t)funct3 << 12) | (imm4_0 << 7) | opcode;
}
uint32_t encodeSB(int32_t imm, uint8_t rs2, uint8_t rs1, uint8_t funct3, uint8_t opcode) {
    // imm in bytes, branch immediate encoded as imm[12|10:5|4:1|11] with LSB 0
    uint32_t imm13 = (uint32_t)imm & 0x1FFF;
    uint32_t imm12 = (imm13 >> 12) & 0x1;
    uint32_t imm10_5 = (imm13 >> 5) & 0x3F;
    uint32_t imm4_1 = (imm13 >> 1) & 0xF;
    uint32_t imm11 = (imm13 >> 11) & 0x1;
    return (imm12 << 31) | (imm10_5 << 25) | ((uint32_t)rs2 << 20) | ((uint32_t)rs1 << 15) | ((uint32_t)funct3 << 12) | (imm4_1 << 8) | (imm11 << 7) | opcode;
}
uint32_t encodeU(int32_t imm, uint8_t rd, uint8_t opcode) {
    uint32_t imm20 = (uint32_t)imm & 0xFFFFF000;
    return imm20 | ((uint32_t)rd << 7) | opcode;
}
uint32_t encodeUJ(int32_t imm, uint8_t rd, uint8_t opcode) {
    uint32_t imm21 = (uint32_t)imm & 0x1FFFFF;
    uint32_t bit20 = (imm21 >> 20) & 1;
    uint32_t bits10_1 = (imm21 >> 1) & 0x3FF;
    uint32_t bit11 = (imm21 >> 11) & 1;
    uint32_t bits19_12 = (imm21 >> 12) & 0xFF;
    return (bit20 << 31) | (bits10_1 << 21) | (bit11 << 20) | (bits19_12 << 12) | ((uint32_t)rd << 7) | opcode;
}

/* Opcode/funct mapping for required instructions (numeric) */
struct OpInfo { uint8_t opcode; uint8_t funct3; uint8_t funct7; };
static unordered_map<string, OpInfo> opmap = {
    // R-format (opcode 0x33)
    {"add", {0x33, 0x0, 0x00}},
    {"sub", {0x33, 0x0, 0x20}},
    {"and", {0x33, 0x7, 0x00}},
    {"or",  {0x33, 0x6, 0x00}},
    {"sll", {0x33, 0x1, 0x00}},
    {"slt", {0x33, 0x2, 0x00}},
    {"sra", {0x33, 0x5, 0x20}},
    {"srl", {0x33, 0x5, 0x00}},
    {"xor", {0x33, 0x4, 0x00}},
    {"mul", {0x33, 0x0, 0x01}}, // M-extension
    // I-format immediate arithmetic (opcode 0x13)
    {"addi", {0x13, 0x0, 0x00}},
    {"andi", {0x13, 0x7, 0x00}},
    {"ori",  {0x13, 0x6, 0x00}},
    // loads (opcode 0x03)
    {"lb", {0x03, 0x0, 0x00}},
    {"lh", {0x03, 0x1, 0x00}},
    {"lw", {0x03, 0x2, 0x00}},
    {"ld", {0x03, 0x3, 0x00}},
    // stores (opcode 0x23)
    {"sb", {0x23, 0x0, 0x00}},
    {"sh", {0x23, 0x1, 0x00}},
    {"sw", {0x23, 0x2, 0x00}},
    {"sd", {0x23, 0x3, 0x00}},
    // branches (opcode 0x63)
    {"beq", {0x63, 0x0, 0x00}},
    {"bne", {0x63, 0x1, 0x00}},
    {"blt", {0x63, 0x4, 0x00}},
    {"bge", {0x63, 0x5, 0x00}},
    // jumps
    {"jal", {0x6F, 0x0, 0x00}},   // UJ
    {"jalr", {0x67, 0x0, 0x00}},  // I
    // U-type
    {"lui", {0x37, 0x0, 0x00}},
    {"auipc", {0x17, 0x0, 0x00}}
};

static string regNameList[] = {}; // not used here but helpful later

pair<uint32_t, string> encodeInstruction(const vector<string>& tokens, uint32_t pc, const SymbolTable &sym) {
    if (tokens.empty()) return {0u, ""};
    string op = lower(tokens[0]);
    if (op == ".word" || op == ".byte" || op == ".asciz" || op == ".dword" || op == ".half") {
        // directives handled separately
        return {0u, ""}
    ;}

    // R-type
    if (opmap.find(op) != opmap.end()) {
        OpInfo info = opmap[op];
        // decide format from opcode (simple heuristic)
        if (info.opcode == 0x33) {
            // R-type: op rd, rs1, rs2
            if (tokens.size() < 4) return {0u, "err: args"};
            uint8_t rd = (uint8_t)regToNum(tokens[1]);
            uint8_t rs1 = (uint8_t)regToNum(tokens[2]);
            uint8_t rs2 = (uint8_t)regToNum(tokens[3]);
            uint32_t inst = encodeR(info.funct7, rs2, rs1, info.funct3, rd, info.opcode);
            stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2] << "," << tokens[3];
            return {inst, ss.str()};
        }
        else if (info.opcode == 0x13 || info.opcode == 0x03 || info.opcode == 0x67) {
            // I-type: many forms
            if (op == "jalr") {
                // jalr rd, imm(rs1)  OR jalr rd, rs1, imm  (spec allow rd, rs1, imm)
                if (tokens.size() == 3 && tokens[2].find('(') != string::npos) {
                    // rd, imm(rs1) e.g. jalr x0, 0(x1)
                    uint8_t rd = (uint8_t)regToNum(tokens[1]);
                    // parse imm(rs1)
                    string tok = tokens[2];
                    size_t p = tok.find('(');
                    string immStr = tok.substr(0,p);
                    string rs1s = tok.substr(p+1, tok.find(')') - p - 1);
                    int32_t imm = parseImmediate(immStr);
                    uint8_t rs1 = (uint8_t)regToNum(rs1s);
                    uint32_t inst = encodeI(imm, rs1, info.funct3, rd, info.opcode);
                    stringstream ss; ss << "jalr " << tokens[1] << "," << tokens[2];
                    return {inst, ss.str()};
                } else if (tokens.size() >= 3) {
                    // jalr rd rs1 imm
                    uint8_t rd = (uint8_t)regToNum(tokens[1]);
                    uint8_t rs1 = (uint8_t)regToNum(tokens[2]);
                    int32_t imm = (tokens.size()>3) ? parseImmediate(tokens[3]) : 0;
                    uint32_t inst = encodeI(imm, rs1, info.funct3, rd, info.opcode);
                    stringstream ss; ss << "jalr " << tokens[1] << "," << tokens[2] << "," << (tokens.size()>3?tokens[3]:"0");
                    return {inst, ss.str()};
                }
                return {0u, "err"};
            } else if (info.opcode == 0x03) {
                // loads: ld rd, imm(rs1)
                if (tokens.size() >= 3 && tokens[2].find('(') != string::npos) {
                    uint8_t rd = (uint8_t)regToNum(tokens[1]);
                    string tok = tokens[2];
                    size_t p = tok.find('(');
                    string immStr = tok.substr(0,p);
                    string rs1s = tok.substr(p+1, tok.find(')') - p - 1);
                    int32_t imm = parseImmediate(immStr);
                    uint8_t rs1 = (uint8_t)regToNum(rs1s);
                    uint32_t inst = encodeI(imm, rs1, info.funct3, rd, info.opcode);
                    stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2];
                    return {inst, ss.str()};
                }
            } else {
                // I-type arithmetic: addi rd, rs1, imm  OR addi rd, imm (if used as move)
                if (tokens.size() >= 4) {
                    uint8_t rd = (uint8_t)regToNum(tokens[1]);
                    uint8_t rs1 = (uint8_t)regToNum(tokens[2]);
                    int32_t imm = parseImmediate(tokens[3]);
                    uint32_t inst = encodeI(imm, rs1, info.funct3, rd, info.opcode);
                    stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2] << "," << tokens[3];
                    return {inst, ss.str()};
                } else if (tokens.size()==3) {
                    // maybe mov style: addi rd, rs1 (imm omitted) - treat as rd, rs1, 0
                    uint8_t rd = (uint8_t)regToNum(tokens[1]);
                    uint8_t rs1 = (uint8_t)regToNum(tokens[2]);
                    uint32_t inst = encodeI(0, rs1, info.funct3, rd, info.opcode);
                    stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2];
                    return {inst, ss.str()};
                }
            }
        } else if (info.opcode == 0x23) {
            // S-type stores: sw rs2, imm(rs1)
            if (tokens.size() >= 3 && tokens[2].find('(') != string::npos) {
                uint8_t rs2 = (uint8_t)regToNum(tokens[1]);
                string tok = tokens[2];
                size_t p = tok.find('(');
                string immStr = tok.substr(0,p);
                string rs1s = tok.substr(p+1, tok.find(')') - p - 1);
                int32_t imm = parseImmediate(immStr);
                uint8_t rs1 = (uint8_t)regToNum(rs1s);
                uint32_t inst = encodeS(imm, rs2, rs1, info.funct3, info.opcode);
                stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2];
                return {inst, ss.str()};
            }
        } else if (info.opcode == 0x63) {
            // branches: beq rs1, rs2, label
            if (tokens.size() >= 4) {
                uint8_t rs1 = (uint8_t)regToNum(tokens[1]);
                uint8_t rs2 = (uint8_t)regToNum(tokens[2]);
                string label = tokens[3];
                uint32_t target = sym.get(label);
                // branch offset = target - pc (pc is address of this instruction)
                int32_t offset = (int32_t)target - (int32_t)pc;
                uint32_t inst = encodeSB(offset, rs2, rs1, info.funct3, info.opcode);
                stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2] << "," << tokens[3];
                return {inst, ss.str()};
            }
        } else if (info.opcode == 0x6F) {
            // jal rd, label
            if (tokens.size() >= 3) {
                uint8_t rd = (uint8_t)regToNum(tokens[1]);
                string label = tokens[2];
                uint32_t tgt = sym.get(label);
                int32_t offset = (int32_t)tgt - (int32_t)pc;
                uint32_t inst = encodeUJ(offset, rd, info.opcode);
                stringstream ss; ss << "jal " << tokens[1] << "," << tokens[2];
                return {inst, ss.str()};
            }
        } else if (info.opcode == 0x37 || info.opcode == 0x17) {
            // U-type: lui rd, imm
            if (tokens.size() >= 3) {
                uint8_t rd = (uint8_t)regToNum(tokens[1]);
                int32_t imm = parseImmediate(tokens[2]) << 12; // imm top bits
                uint32_t inst = encodeU(imm, rd, info.opcode);
                stringstream ss; ss << op << " " << tokens[1] << "," << tokens[2];
                return {inst, ss.str()};
            }
        }
    }

    // instruction not recognized
    return {0u, string("unknown: ") + (tokens.empty()?string(""):tokens[0])};
}
