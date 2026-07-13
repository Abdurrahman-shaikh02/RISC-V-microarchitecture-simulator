#include <elf.h>
int elf_loader(const char *filename, uint8_t *dram, uint32_t dram_size, char ** instructions, uint32_t total_number_of_instructions, uint32_t *entry_point);

