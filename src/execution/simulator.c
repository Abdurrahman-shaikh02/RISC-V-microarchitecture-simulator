#include "header.h"
#include "control.h"
#include "pipeline.h"
#include "memory.h"
#include "internal_memory.h"
#include "utility.h"
#include "cache.h"
#include "stats.h"
#include "simulator.h"


void simulate(char * path, uint32_t entry, int visuals){
	FILE * fd = NULL;

	if(visuals == 1){
		if(path == NULL){
			fd = stdout;
		}else{
			fd = fopen(path, "w");
		}


		if(fd == NULL){
			log_fatal("Couldnt open file for pipeline visuals");
			exit(1);
		}
	}

	int pause_counter = -1;		//this is to count 1 cycles from the cycle where ebreak was in exec
	int trap_counter = -1;		//this is to count 1 cycles from the cycle where ecall was in exec
	int halt = 0;

	pc = entry;	//set pc to entry point
	
	//start machine
	while(1){
		log_break();
		log_info("New cycle starts");
		char str[65];
		snprintf(str, sizeof(str), "%" PRIu64, cycle + 1);
		log_info(str);

		//wb stage
		wb_stage();

		//ma stage
		ma_stage();
		if(mfc == 0){
			//ex stage stall
			
			//clearing the next pipeline register...
			ma_wb = (MA_WB){0, 0, {0}, "                    "};
			//updating decode stage operands
			update_operands();

			//increase the clock, handle visuals and skip the IF, ID, EX stages(stall) and continue to next cycle
			cycle++;
			if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
			continue;
		}else if(FLUSH == 1){
			//clear ex_ma, id_ex, if_id stages
			//and let the execution continue... because this flush and pc change should have been done at the end of the last clock cycle...
			//but due to our design we have decided to do it right now because the ex, id, if of the current cycle havent been completed yet...
			ex_ma = (EX_MA){0, 0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
			id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};
			if_id = (IF_ID){0, 0, 0, "                    "};

			//cancelling any on going read instruction operations going on !!!
			read_memory_i(1);
			reset_decode_flags();
			ex_flushes++;	//stat

			FLUSH = 0;
		}else if(STRUCTURAL_HAZARD_STALL == 1){
			//the memory access has failed due to a pending memory access from the if stage
			//clear the next pipeline register ma_wb
			ma_wb = (MA_WB){0, 0, {0}, "                    "};
			//updating decode stage operands
			update_operands();

			//call if stage to make its attempt on the memory access... and update its internal stall counter.
			if_stage();
			if(mfc_i == 0){
				//if stage stall

				//clearing the next pipeline rgister...
				if_id = (IF_ID){0, 0, 0, "                    "};
			}
	
			STRUCTURAL_HAZARD_STALL = 0;

			//increase the clock, handle visuals and skip the IF, ID, EX stages(stall) and continue to next cycle
			cycle++;
			if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
			continue;
		}

		//ex stage
		ex_stage();
		if(PAUSE == 1){
			log_debug("ebreak encountered");
			//all the following codes job is to stall the pipeline once for the last instruction to get completed...
			if(pause_counter == 0){
				log_info("Stopping for ebreak");
				//complete the clock cycle (no stalling) let the ebreak get erased from the id_ex reg then break
				
				//id_stage
				id_stage();
				if(HAZARD_STALL == 1){
					//clear id_ex register
					//increment cycle, print visuals, reset HAZARD_STALL

					id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};

					cycle++;
					if(visuals == 1)
						fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

					HAZARD_STALL = 0;
					continue;
				}else if(FLUSH == 1){
					//clear id_ex register, if_id register... let the excution continue
					id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};
					if_id = (IF_ID){0, 0, 0, "                    "};

					//cancelling any on going read instruction operations going on !!!
					read_memory_i(1);
					
					id_flushes++;	//stat

					FLUSH = 0;
				}


				//if_stage
				if_stage();
				if(mfc_i == 0){
					//if stage stall

					//clearing the next pipeline rgister...
					if_id = (IF_ID){0, 0, 0, "                    "};

					cycle++;
					if(visuals == 1)
						fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
					continue;
				}
				cycle++;
				if(visuals == 1)
					fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
				
				pause_counter--;
				PAUSE = 0;
				log_debug("PAUSE flag set to 0");
				break;
			}
			/*else if(pause_counter > 0){
				//stall(skip the IF, ID stage) and decrement the pause_counter
				
				//clearing ex_ma register
				ex_ma = (EX_MA){0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
				
				//increase the clock, handle visuals, handle pause_counter and skip the IF, ID stages(stall) and continue to next cycle
				pause_counter--;
				cycle++;
				if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

				PAUSE = 0;
				log_debug("PAUSE flag set to 0");
				continue;
			}*/
			else{
				//pause_counter < 0 means we have just encountered an ebreak need to stall it...
				log_debug("pause_counter set to 1");
				pause_counter = 1;
				//stall(skip the IF, ID stage) and decrement the pause_counter
				
				//clearing ex_ma register
				ex_ma = (EX_MA){0, 0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
				
				//increase the clock, handle visuals, handle pause_counter and skip the IF, ID stages(stall) and continue to next cycle
				pause_counter--;
				cycle++;
				if(visuals == 1)
					fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

				PAUSE = 0;
				log_debug("PAUSE flag set to 0");
				continue;
			}
		}else if(TRAP == 1){
			//all the following codes job is to stall the pipeline once for the last instruction to get completed...
			if(trap_counter == 0){
				log_info("Trapping for ecall");
				//complete the clock cycle (no stalling) let the ecall get erased from the id_ex reg then break
				
				//id_stage
				id_stage();
				if(HAZARD_STALL == 1){
					//clear id_ex register
					//increment cycle, print visuals, reset HAZARD_STALL

					id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};

					cycle++;
					if(visuals == 1)
						fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

					HAZARD_STALL = 0;
					continue;
				}else if(FLUSH == 1){
					//clear id_ex register, if_id register... let the excution continue
					id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};
					if_id = (IF_ID){0, 0, 0, "                    "};

					//cancelling any on going read instruction operations going on !!!
					read_memory_i(1);

					id_flushes++;	//stat

					FLUSH = 0;
				}


				//if_stage
				if_stage();
				if(mfc_i == 0){
					//if stage stall

					//clearing the next pipeline rgister...
					if_id = (IF_ID){0, 0, 0, "                    "};

					cycle++;
					if(visuals == 1)
						fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
					continue;
				}
				cycle++;
				if(visuals == 1)
					fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
				
				//do the system call here
				uint32_t ecall_code = reg_file[17];
				trap_counter--;
				TRAP = 0;
				log_debug("TRAP flag was set to 0");

				if(ecall_code == SYSTEM_HALT_CODE){
					halt = 1;
					break;
				}

				continue;
			}
			/*else if(trap_counter > 0){
				//stall(skip the IF, ID stage) and decrement the trap_counter
				
				//clearing ex_ma register
				ex_ma = (EX_MA){0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
				
				//increase the clock, handle visuals, handle trap_counter and skip the IF, ID stages(stall) and continue to next cycle
				trap_counter--;
				cycle++;
				if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

				TRAP = 0;
				log_debug("TRAP flag was set to 0");
				continue;
			}*/
			else{
				//trap_counter < 0 means we have just encountered an ebreak need to stall it...
				log_debug("trap_counter was set to 1");
				trap_counter = 1;
				//stall(skip the IF, ID stage) and decrement the pause_counter
				
				//clearing ex_ma register
				ex_ma = (EX_MA){0, 0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
				
				//increase the clock, handle visuals, handle pause_counter and skip the IF, ID stages(stall) and continue to next cycle
				trap_counter--;
				cycle++;
				if(visuals == 1)
					fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

				TRAP = 0;
				log_debug("TRAP flag was set to 0");
				continue;
			}
		}
		else if(FORWARDING_STALL == 1){
			//compulsory stall...
			
			//clearing ex_ma register
			ex_ma = (EX_MA){0, 0, 0, 0, 0, {0, 0, 0, 0}, {0}, "                    "};
			update_operands();	//updating decode stage operands

			cycle++;
			if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);

			FORWARDING_STALL = 0;
			continue;
		}


		//id_stage
		id_stage();
		if(HAZARD_STALL == 1){
			//clear id_ex register
			//increment cycle, print visuals, reset HAZARD_STALL

			id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};

			cycle++;
			if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
			
			HAZARD_STALL = 0;
			continue;
		}else if(FLUSH == 1){
			//clear id_ex register, if_id register... let the excution continue
			id_ex = (ID_EX){0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0},{0,0,0,0},{0}, "                    "};
			if_id = (IF_ID){0, 0, 0, "                    "};

			//cancelling any on going read instruction operations going on !!!
			read_memory_i(1);

			id_flushes++;	//stat

			FLUSH = 0;
		}

		//if_stage
		if_stage();
		if(mfc_i == 0){
			//if stage stall

			//clearing the next pipeline rgister...
			if_id = (IF_ID){0, 0, 0, "                    "};

			cycle++;
			if(visuals == 1)
				fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
			continue;
		}

		
		cycle++;

		if(visuals == 1)
			fprintf(fd, "Cycle %ld : %s | %s | %s | %s | %s\n", cycle,  if_id.ins, id_ex.ins, ex_ma.ins, ma_wb.ins, wb_if.ins);
	}

	//if(halt) exit(0);
	
	if(visuals == 1 && path != NULL){
		fclose(fd);
	}
}
