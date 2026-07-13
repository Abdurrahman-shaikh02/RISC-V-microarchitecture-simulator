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

int main(){
	//initialize memory...
	
	printf("Confinguring System...\n");
	configure(NULL);

	printf("Initializing Memory...\n");
	custom_loader("output.txt", NULL);
	
	printf("Running Simulation...\n");
	simulate("pipevis.txt", 1);		//dont disable printfs until ur sure	main thing to take care of is free_instructions and the instruction retired count
	
	//were allocated in init_memory
	free_instructions();

	printf("Dumping Machine State...\n");
	display_general_purpose_registers("register_dump");
	//display_internal_registers(NULL);
	display_memory("memory_dump.txt");

	printf("Generating Stats...\n");
	display_stats(NULL);
	
	//printf("%ld\n", cycle);
	
	//close the files tooooooo !!!
}
