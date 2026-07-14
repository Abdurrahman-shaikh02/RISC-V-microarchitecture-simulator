/*
The IF stage function's success must be determined by the value of mfc_i
If mfc_i is 1 then the stage was successful, otherwise it wasnt.
If it wasnt then we must insert a bubble ie clear the if_id register.

need to declare internal registers, pipeline registers here.
*/

#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "utility.h"
#include "cache.h"
#include "component_declaration.h"
#include "stats.h"
#include "configure.h"
#include "simulator.h"
#include "custom_loader.h"
#include "elf_loader.h"

int main(int argc, char * argv[]){

	int pip = 0;		// -p	pipeline visuals
	int gen_trace = 0;      // -t	memory trace files
	int mem_dump = 0;	// -m	dump memory
	int reg_dump = 0;	// -r	dump registers
	int log = 0;		// -l	generate log file

	char *custom_object = NULL; // -a <file>
	char *elf_object    = NULL; // -e <file>
	char *config        = NULL; // -c <file>

	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "-p") == 0){
			if (pip) {
				fprintf(stderr, "Error: duplicate -p flag\n");
				return EXIT_FAILURE;
			}
			pip = 1;
		}else if (strcmp(argv[i], "-t") == 0){
			if (gen_trace) {
				fprintf(stderr, "Error: duplicate -t flag\n");
				return EXIT_FAILURE;
			}
			gen_trace = 1;
		}else if (strcmp(argv[i], "-m") == 0){
			if (mem_dump) {
				fprintf(stderr, "Error: duplicate -m flag\n");
				return EXIT_FAILURE;
			}
			mem_dump = 1;
		}else if (strcmp(argv[i], "-r") == 0){
			if (reg_dump) {
				fprintf(stderr, "Error: duplicate -r flag\n");
				return EXIT_FAILURE;
			}
			reg_dump = 1;
		}else if (strcmp(argv[i], "-l") == 0){
			if (log) {
				fprintf(stderr, "Error: duplicate -l flag\n");
				return EXIT_FAILURE;
			}
			log = 1;
		}else if (strcmp(argv[i], "-a") == 0){

			if (elf_object != NULL) {
				fprintf(stderr, "Error: already supplied elf object file\n");
				return EXIT_FAILURE;
			}

			if (custom_object != NULL) {
				fprintf(stderr, "Error: duplicate -a flag\n");
				return EXIT_FAILURE;
			}

			if (i + 1 >= argc || argv[i + 1][0] == '-') {
				fprintf(stderr, "Error: -a requires a filename\n");
				return EXIT_FAILURE;
			}

			custom_object = argv[++i];

		}else if (strcmp(argv[i], "-e") == 0){

			if (custom_object != NULL) {
				fprintf(stderr, "Error: already supplied custom object file\n");
				return EXIT_FAILURE;
			}

			if (elf_object != NULL) {
				fprintf(stderr, "Error: duplicate -e flag\n");
				return EXIT_FAILURE;
			}

			if (i + 1 >= argc || argv[i + 1][0] == '-') {
				fprintf(stderr, "Error: -e requires a filename\n");
				return EXIT_FAILURE;
			}

			elf_object = argv[++i];

		}else if (strcmp(argv[i], "-c") == 0){

			if (config != NULL) {
				fprintf(stderr, "Error: duplicate -c flag\n");
				return EXIT_FAILURE;
			}

			if (i + 1 >= argc || argv[i + 1][0] == '-') {
				fprintf(stderr, "Error: -c requires a filename\n");
				return EXIT_FAILURE;
			}

			config = argv[++i];
			printf("Config file not supported yet !\n");
			return 0;
		}else{
			fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
			return EXIT_FAILURE;
		}
	}
	/*
	   printf("pipeline      : %d\n", pip);
	   printf("gen_trace         : %d\n", gen_trace);
	   printf("mem_dump      : %d\n", mem_dump);
	   printf("reg_dump      : %d\n", reg_dump);
	   printf("log           : %d\n", log);
	   printf("custom object : %s\n", custom_object ? custom_object : "(none)");
	   printf("elf object    : %s\n", elf_object ? elf_object : "(none)");
	   printf("config        : %s\n", config ? config : "(none)");
	   */
	//initialize memory...
	

	if(log == 0){
		CURRENT_LEVEL = 6;	//allowing no logs to be made
	}


	printf("Configuring System...\n");
	configure(config);


	printf("Initializing Memory...\n");
	if(gen_trace){
		trace = 1;
	}
	open_trace_files("instruction_trace.txt", "data_trace.txt");
	uint32_t entry;
	if(elf_object != NULL){
		if(elf_loader(elf_object, dram, DRAM_SIZE, instructions, total_number_of_instructions, &entry) != 0){
			log_fatal("Elf loader failed.");
			exit(1);
		}
	}else if(custom_object != NULL){
		if(custom_loader(custom_object, NULL, &entry) != 0){
			log_fatal("Custom loader failed.");
			exit(1);
		}
	}else{
		fprintf(stderr, "Input file not specified\n");
		return EXIT_FAILURE;
	}
	

	printf("Running Simulation...\n");
	simulate("pipevis.txt", entry, pip);		//dont disable printfs until ur sure	main thing to take care of is free_instructions and the instruction retired count
	
	//were allocated in init_memory
	free_instructions();


	printf("Dumping Machine State...\n");
	if(reg_dump){
		display_general_purpose_registers("register_dump");
		display_internal_registers("internal_register_dump");
	}
	if(mem_dump){
		display_memory("memory_dump.txt");
	}


	printf("Generating Stats...\n");
	display_stats(NULL);
	
	//printf("%ld\n", cycle);
	
	//close the files tooooooo !!!
}
