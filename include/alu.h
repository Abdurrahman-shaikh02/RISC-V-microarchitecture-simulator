uint32_t arithmetic_unit(uint32_t operand1, uint32_t operand2, uint32_t opcode);
uint32_t logical_unit(uint32_t operand1, uint32_t operand2, uint32_t opcode, uint8_t sign);
uint32_t less_than_comparator(uint32_t operand1, uint32_t operand2, uint8_t sign);
uint32_t branch_comparator(uint32_t operand1, uint32_t operand2, uint8_t branch_opcode, uint8_t sign);
uint32_t generate_immediate(char format, uint32_t ir);
