#ifndef ENCODER_H
#define ENCODER_H
#include <string>
#include <cstdint>
#include <vector>
#include "symbols.h"

uint32_t encodeR(uint8_t funct7, uint8_t rs2, uint8_t rs1, uint8_t funct3, uint8_t rd, uint8_t opcode);
uint32_t encodeI(int32_t imm, uint8_t rs1, uint8_t funct3, uint8_t rd, uint8_t opcode);
uint32_t encodeS(int32_t imm, uint8_t rs2, uint8_t rs1, uint8_t funct3, uint8_t opcode);
uint32_t encodeSB(int32_t imm, uint8_t rs2, uint8_t rs1, uint8_t funct3, uint8_t opcode);
uint32_t encodeU(int32_t imm, uint8_t rd, uint8_t opcode);
uint32_t encodeUJ(int32_t imm, uint8_t rd, uint8_t opcode);

// top-level: pass tokens and current pc, symbol table to resolve labels
// returns pair(machineCode, comment)
std::pair<uint32_t, std::string> encodeInstruction(const std::vector<std::string>& tokens, uint32_t pc, const SymbolTable &sym);

#endif
