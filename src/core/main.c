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
#include "component_declaration.h"


int main(){
	//initialize memory...
	init_memory_x("output.txt");

	//then while loop...
	int cycle = 0;
	char c;
	while(1){
		log_break();
		log_info("New cycle starts");

		//wb stage
		wb_stage();

		//ma stage
		ma_stage();
		if(mfc == 0){
			//ex stage stall
			
			//clearing the next pipeline register...
			ma_wb = (MA_WB){0, 0, {0}};

			cycle++;
			continue;
		}

		//ex stage
		ex_stage();
		if(PAUSE == 1){
			cycle++;
			break;
		}

		//id_stage
		id_stage();

		//if_stage
		if_stage();
		if(mfc_i == 0){
			//if stage stall
			
			//clearing the next pipeline rgister...
			if_id = (IF_ID){0, 0, 0};

			cycle++;
			continue;
		}

		cycle++;

		//printf("%s | %s | %s | %s | %s\n", if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

	}

	display_general_purpose_registers();
	//display_internal_registers();
	display_memory();
	
	//printf("%d\n", cycle);
	
	free_instructions();
}
