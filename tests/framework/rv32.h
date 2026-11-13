// RV32I instruction encoders, written from the ISA specification.
// They are deliberately independent from the project's assembler so that
// both the assembler and the simulator can be checked against them.
#ifndef RV32_H
#define RV32_H

#include <stdint.h>

enum {
	OP_LUI = 0x37, OP_AUIPC = 0x17, OP_JAL = 0x6F, OP_JALR = 0x67, OP_BRANCH = 0x63,
	OP_LOAD = 0x03, OP_STORE = 0x23, OP_IMM = 0x13, OP_REG = 0x33, OP_FENCE = 0x0F, OP_SYSTEM = 0x73
};

static inline uint32_t enc_r(uint32_t funct7, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode){
	return (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

static inline uint32_t enc_i(int32_t imm, uint32_t rs1, uint32_t funct3, uint32_t rd, uint32_t opcode){
	return (((uint32_t)imm & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
}

static inline uint32_t enc_s(int32_t imm, uint32_t rs2, uint32_t rs1, uint32_t funct3, uint32_t opcode){
	uint32_t u = (uint32_t)imm;
	return (((u >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | ((u & 0x1F) << 7) | opcode;
}

static inline uint32_t enc_b(int32_t imm, uint32_t rs2, uint32_t rs1, uint32_t funct3){
	uint32_t u = (uint32_t)imm;
	return (((u >> 12) & 1) << 31) | (((u >> 5) & 0x3F) << 25) | (rs2 << 20) | (rs1 << 15) |
		(funct3 << 12) | (((u >> 1) & 0xF) << 8) | (((u >> 11) & 1) << 7) | OP_BRANCH;
}

static inline uint32_t enc_u(uint32_t imm20, uint32_t rd, uint32_t opcode){
	return ((imm20 & 0xFFFFF) << 12) | (rd << 7) | opcode;
}

static inline uint32_t enc_j(int32_t imm, uint32_t rd){
	uint32_t u = (uint32_t)imm;
	return (((u >> 20) & 1) << 31) | (((u >> 1) & 0x3FF) << 21) | (((u >> 11) & 1) << 20) |
		(((u >> 12) & 0xFF) << 12) | (rd << 7) | OP_JAL;
}

#endif
