// src/utils/custom_loader.c (assembler object files) and src/utils/elf_loader.c
#include "sim_support.h"
#include <stddef.h>

//------------------------------------------------------------------------------
// custom loader: the verbose object format written by ./assemble -v
//------------------------------------------------------------------------------

TEST(custom_loader_loads_words_little_endian){
	sim_setup(-1);
	test_write_file("p.obj",
		"[0] 00000000   00c00093           addi    r1, r0, 12\n"
		"[1] 0x000004   00500113           addi    r2, r0, 5\n"
		"[2] 0x000008   00000073           ecall\n");
	uint32_t entry = 123;
	CHECK_EQ(custom_loader("p.obj", NULL, &entry), 0);
	CHECK_EQ(entry, 0);
	CHECK_EQ(sim_dram_word(0), 0x00c00093);
	CHECK_EQ(sim_dram_word(4), 0x00500113);
	CHECK_EQ(sim_dram_word(8), 0x00000073);
	CHECK_EQ(dram[0], 0x93);
	CHECK_EQ(sim_dram_word(12), 0);
}

TEST(custom_loader_keeps_20_char_instruction_strings){
	sim_setup(-1);
	test_write_file("p.obj",
		"[0] 00000000   00c00093           addi    r1, r0, 12\n"
		"[1] 0x000004   00000073   ecall\n"
		"[2] 0x000008   00000073   a_very_long_source_line_that_is_cut\n");
	uint32_t entry;
	custom_loader("p.obj", NULL, &entry);
	CHECK_STR_EQ(instructions[0], "addi    r1, r0, 12  ");
	CHECK_STR_EQ(instructions[1], "ecall               ");
	CHECK_STR_EQ(instructions[2], "a_very_long_source_l");
	CHECK(instructions[3] == NULL);
}

TEST(custom_loader_skips_label_and_junk_lines){
	sim_setup(-1);
	test_write_file("p.obj",
		"loop:\n"
		"\n"
		"[0] 00000000   00100093   addi r1, r0, 1\n"
		"end:\n"
		"[1] 0x000004   00000073   ecall\n");
	uint32_t entry;
	custom_loader("p.obj", NULL, &entry);
	CHECK_EQ(sim_dram_word(0), 0x00100093);
	CHECK_EQ(sim_dram_word(4), 0x00000073);
	CHECK_STR_EQ(instructions[1], "ecall               ");
}

TEST(custom_loader_places_words_sequentially_ignoring_address_column){
	sim_setup(-1);
	test_write_file("p.obj",
		"[0] 0x000100   00100093   a\n"
		"[1] 0x000200   00200093   b\n");
	uint32_t entry;
	custom_loader("p.obj", NULL, &entry);
	CHECK_EQ(sim_dram_word(0), 0x00100093);
	CHECK_EQ(sim_dram_word(4), 0x00200093);
}

TEST(custom_loader_missing_file_is_fatal){
	sim_setup(-1);
	uint32_t entry;
	CHECK_FATAL(custom_loader("nope.obj", NULL, &entry), "failed to open memory image file");
}

TEST(custom_loader_program_too_large_is_fatal){
	sim_setup(-1);
	text_segment_limit = 15;	// room for 3 words (addr + 3 >= limit check)
	FILE * f = fopen("p.obj", "w");
	for(int i = 0; i < 8; i++) fprintf(f, "[%d] 0x%06x   00000013   nop\n", i, i * 4);
	fclose(f);
	uint32_t entry;
	CHECK_FATAL(custom_loader("p.obj", NULL, &entry), "program too large for memory");
}

//------------------------------------------------------------------------------
// ELF loader
//------------------------------------------------------------------------------

typedef struct {
	Elf32_Ehdr eh;
	Elf32_Phdr ph[3];
	uint8_t text[16];
	uint8_t data[8];
} tiny_elf;

static tiny_elf make_elf(void){
	tiny_elf e;
	memset(&e, 0, sizeof(e));
	memcpy(e.eh.e_ident, ELFMAG, SELFMAG);
	e.eh.e_ident[EI_CLASS] = ELFCLASS32;
	e.eh.e_ident[EI_DATA] = ELFDATA2LSB;
	e.eh.e_ident[EI_VERSION] = EV_CURRENT;
	e.eh.e_type = ET_EXEC;
	e.eh.e_machine = EM_RISCV;
	e.eh.e_version = EV_CURRENT;
	e.eh.e_entry = 0x8;
	e.eh.e_phoff = offsetof(tiny_elf, ph);
	e.eh.e_ehsize = sizeof(Elf32_Ehdr);
	e.eh.e_phentsize = sizeof(Elf32_Phdr);
	e.eh.e_phnum = 3;

	// text segment at 0
	e.ph[0].p_type = PT_LOAD;
	e.ph[0].p_offset = offsetof(tiny_elf, text);
	e.ph[0].p_vaddr = 0;
	e.ph[0].p_filesz = 16;
	e.ph[0].p_memsz = 16;
	e.ph[0].p_flags = PF_R | PF_X;
	uint32_t code[4] = {0x00000013, 0x00000013, 0x00100093, 0x00000073};
	memcpy(e.text, code, 16);

	// non loadable segment, must be ignored
	e.ph[1].p_type = PT_NOTE;
	e.ph[1].p_offset = offsetof(tiny_elf, data);
	e.ph[1].p_vaddr = 0x100;
	e.ph[1].p_filesz = 8;
	e.ph[1].p_memsz = 8;

	// data + bss at 0x40000: 8 bytes from the file, 8 more zeroed
	e.ph[2].p_type = PT_LOAD;
	e.ph[2].p_offset = offsetof(tiny_elf, data);
	e.ph[2].p_vaddr = 0x40000;
	e.ph[2].p_filesz = 8;
	e.ph[2].p_memsz = 16;
	e.ph[2].p_flags = PF_R | PF_W;
	uint8_t d[8] = {1, 2, 3, 4, 5, 6, 7, 8};
	memcpy(e.data, d, 8);
	return e;
}

static void write_elf(const char * path, const tiny_elf * e, size_t size){
	FILE * f = fopen(path, "wb");
	fwrite(e, 1, size, f);
	fclose(f);
}

static int load(const char * path, uint32_t * entry){
	return elf_loader(path, dram, DRAM_SIZE, instructions, total_number_of_instructions, entry);
}

TEST(elf_loads_segments_and_entry){
	sim_setup(-1);
	tiny_elf e = make_elf();
	write_elf("p.elf", &e, sizeof(e));
	memset(dram + 0x40000, 0xAA, 32);	// garbage where the bss goes
	dram[0x100] = 0x55;
	uint32_t entry = 0;
	char out[4096];
	int st;
	RUN_ISOLATED(out, st, { int r = load("p.elf", &entry); TEST_EXIT(r == 0 && entry == 8 ? 0 : 3); });
	CHECK_EQ(st, 0);
	CHECK_CONTAINS(out, "Loaded segment:\n  VA      : 0x00000000\n  FileSz  : 16\n  MemSz   : 16\n  Flags   : 0x5\n");
	CHECK_CONTAINS(out, "  VA      : 0x00040000\n  FileSz  : 8\n  MemSz   : 16\n");

	CHECK_EQ(load("p.elf", &entry), 0);
	CHECK_EQ(entry, 8);
	CHECK_EQ(sim_dram_word(8), 0x00100093);
	CHECK_EQ(sim_dram_word(0x40000), 0x04030201);
	CHECK_EQ(sim_dram_word(0x40004), 0x08070605);
	CHECK_EQ(sim_dram_word(0x40008), 0);		// bss zeroed
	CHECK_EQ(sim_dram_word(0x4000C), 0);
	CHECK_EQ(dram[0x40010], 0xAA);			// nothing past memsz touched
	CHECK_EQ(dram[0x100], 0x55);			// PT_NOTE not loaded
}

TEST(elf_fills_address_instruction_strings){
	sim_setup(-1);
	tiny_elf e = make_elf();
	write_elf("p.elf", &e, sizeof(e));
	uint32_t entry;
	CHECK_EQ(load("p.elf", &entry), 0);
	CHECK_STR_EQ(instructions[0], "     0x00000000     ");
	CHECK_STR_EQ(instructions[3], "     0x0000000c     ");
	CHECK_STR_EQ(instructions[total_number_of_instructions - 1], "     0x0001fffc     ");
	CHECK_EQ(strlen(instructions[5]), 20);
}

#define CHECK_ELF_REJECTED(mutation, size, msg) do { \
	sim_setup(-1); \
	tiny_elf e = make_elf(); \
	mutation; \
	write_elf("bad.elf", &e, size); \
	uint32_t entry_; \
	char out_[4096]; \
	int st_; \
	RUN_ISOLATED(out_, st_, { TEST_EXIT(load("bad.elf", &entry_) == -1 ? 0 : 3); }); \
	CHECK_MSG(st_ == 0, "%s: loader did not return -1 (status %d)", #mutation, st_); \
	CHECK_CONTAINS(out_, "[FATAL]: " msg); \
} while(0)

TEST(elf_rejects_invalid_files){
	CHECK_ELF_REJECTED(e.eh.e_ident[1] = 'X', sizeof(e), "Not an ELF file.");
	CHECK_ELF_REJECTED(e.eh.e_ident[EI_CLASS] = ELFCLASS64, sizeof(e), "Not a 32-bit ELF.");
	CHECK_ELF_REJECTED(e.eh.e_ident[EI_DATA] = ELFDATA2MSB, sizeof(e), "Not little-endian.");
	CHECK_ELF_REJECTED(e.eh.e_machine = EM_X86_64, sizeof(e), "Not a RISC-V executable.");
	CHECK_ELF_REJECTED(e.eh.e_type = ET_REL, sizeof(e), "ELF is not executable.");
	CHECK_ELF_REJECTED(e.eh.e_phnum = 0, sizeof(e), "No program headers.");
	CHECK_ELF_REJECTED(e.eh.e_phoff = 0, sizeof(e), "No program headers.");
	CHECK_ELF_REJECTED((void)0, 20, "Unable to read ELF header.");
	CHECK_ELF_REJECTED((void)0, sizeof(Elf32_Ehdr) + 8, "Failed to read program headers.");
	CHECK_ELF_REJECTED(e.ph[2].p_filesz = 4096, sizeof(e), "Segment read failed.");
	CHECK_ELF_REJECTED(e.ph[2].p_vaddr = DRAM_SIZE - 8, sizeof(e), "Segment exceeds DRAM.");
}

TEST(elf_missing_file){
	sim_setup(-1);
	uint32_t entry;
	char out[4096];
	int st;
	RUN_ISOLATED(out, st, { TEST_EXIT(load("nope.elf", &entry) == -1 ? 0 : 3); });
	CHECK_EQ(st, 0);
	CHECK_CONTAINS(out, "[FATAL]: Couldnt open ELF file.");
}

TEST_SUITE("loaders",
	T(custom_loader_loads_words_little_endian),
	T(custom_loader_keeps_20_char_instruction_strings),
	T(custom_loader_skips_label_and_junk_lines),
	T(custom_loader_places_words_sequentially_ignoring_address_column),
	T(custom_loader_missing_file_is_fatal),
	T(custom_loader_program_too_large_is_fatal),
	T(elf_loads_segments_and_entry),
	T(elf_fills_address_instruction_strings),
	T(elf_rejects_invalid_files),
	T(elf_missing_file),
)
