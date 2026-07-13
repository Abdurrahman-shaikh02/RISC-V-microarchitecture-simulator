#include "header.h"
#include <string.h>
#include "elf_loader.h"

static int read_elf_header(FILE *fp, Elf32_Ehdr *ehdr);
static int read_program_headers(FILE *fp, const Elf32_Ehdr *ehdr, Elf32_Phdr **phdrs);

static int load_segments(FILE *fp, const Elf32_Phdr *phdrs, int phnum, uint8_t *dram, uint32_t dram_size);

void fill_instruction_strings(char ** instructions, uint32_t total_number_of_instructions){
    for(int i = 0; i < total_number_of_instructions; i++){
        char * s = (char *)malloc(21);
        strcpy(s, "<------------------>");
        instructions[i] = s;
    }
}

int elf_loader(const char *filename, uint8_t *dram, uint32_t dram_size, char ** instructions, uint32_t total_number_of_instructions, uint32_t *entry_point){
    //returns 0 on success, -1 otherwise

    log_info("Loading ELF file.");

    FILE *fp = fopen(filename, "rb");
    if (!fp){
        log_fatal("Couldnt open ELF file.");
        return -1;
    }

    Elf32_Ehdr ehdr;

    if (read_elf_header(fp, &ehdr)){
        fclose(fp);
        return -1;
    }

    Elf32_Phdr *phdrs = NULL;

    if (read_program_headers(fp, &ehdr, &phdrs)){
        fclose(fp);
        return -1;
    }

    if (load_segments(fp, phdrs, ehdr.e_phnum, dram, dram_size)){
        free(phdrs);
        fclose(fp);
        return -1;
    }

    *entry_point = ehdr.e_entry;

    free(phdrs);
    fclose(fp);

    fill_instruction_strings(instructions, total_number_of_instructions);

    return 0;
}


static int read_elf_header(FILE *fp, Elf32_Ehdr *ehdr){
    //return zero upon success, -1 otherwise
    rewind(fp);

    if (fread(ehdr, sizeof(Elf32_Ehdr), 1, fp) != 1){
        log_fatal("Unable to read ELF header.");
        return -1;
    }

    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG)){
        log_fatal("Not an ELF file.");
        return -1;
    }

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32){
        log_fatal("Not a 32-bit ELF.");
        return -1;
    }

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB){
        log_fatal("Not little-endian.");
        return -1;
    }

    if (ehdr->e_machine != EM_RISCV){
        log_fatal("Not a RISC-V executable.");
        return -1;
    }

    if (ehdr->e_type != ET_EXEC){
        log_fatal("ELF is not executable.");
        return -1;
    }

    if (ehdr->e_phoff == 0 || ehdr->e_phnum == 0){
        log_fatal("No program headers.");
        return -1;
    }

    return 0;
}


static int read_program_headers(FILE *fp, const Elf32_Ehdr *ehdr, Elf32_Phdr **phdrs){
    //return zero upon success, -1 otherwise
    *phdrs = malloc(ehdr->e_phnum * sizeof(Elf32_Phdr));

    if (*phdrs == NULL){
        log_fatal("Read pheader malloc failed.");
        return -1;
    }

    if (fseek(fp, ehdr->e_phoff, SEEK_SET)){
        free(*phdrs);
        log_fatal("Read pheader fseek failed.");
        return -1;
    }

    if (fread(*phdrs, sizeof(Elf32_Phdr), ehdr->e_phnum, fp) != ehdr->e_phnum){
        free(*phdrs);
        log_fatal("Failed to read program headers.");
        return -1;
    }

    return 0;
}


static int load_segments(FILE *fp, const Elf32_Phdr *phdrs, int phnum, uint8_t *dram, uint32_t dram_size){
    //return 0 upon success, -1 otherwise
    
    for (int i = 0; i < phnum; i++){
        const Elf32_Phdr *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD) continue;

        if (ph->p_vaddr + ph->p_memsz > dram_size){
            log_fatal("Segment exceeds DRAM.");
            return -1;
        }

        if (fseek(fp, ph->p_offset, SEEK_SET)){
            log_fatal("Load Segment Seek failed.");
            return -1;
        }

        if (fread(dram + ph->p_vaddr, 1, ph->p_filesz, fp) != ph->p_filesz){
            log_fatal("Segment read failed.");
            return -1;
        }

        memset(dram + ph->p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        
        //
        printf("Loaded segment:\n");
        printf("  VA      : 0x%08x\n", ph->p_vaddr);
        printf("  FileSz  : %u\n", ph->p_filesz);
        printf("  MemSz   : %u\n", ph->p_memsz);
        printf("  Flags   : 0x%x\n\n", ph->p_flags);
    }

    return 0;
}
