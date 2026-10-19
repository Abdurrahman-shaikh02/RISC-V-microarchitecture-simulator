// src/pipeline/{if,id,ma,wb}.c: stage behaviour that is not instruction
// semantics (those are in test_ex.c)
#include "sim_support.h"

int set_hazard_stall();

//------------------------------------------------------------------------------
// IF
//------------------------------------------------------------------------------

TEST(if_fetches_and_advances_pc){
	sim_setup(-1);
	uint32_t prog[] = {0x00c00093, 0x00500113};
	sim_load_words(0, prog, 2);
	if_stage();
	CHECK_EQ(mfc_i, COMPLETED);
	CHECK_EQ(if_id.IR, 0x00c00093);
	CHECK_EQ(if_id.PC, 0);
	CHECK_EQ(if_id.PC_next, 4);
	CHECK_EQ(pc, 4);
	CHECK_EQ(ir, 0x00c00093);
	CHECK(if_id.ins == instructions[0]);
	if_stage();
	CHECK_EQ(if_id.IR, 0x00500113);
	CHECK_EQ(if_id.PC, 4);
	CHECK_EQ(pc, 8);
	CHECK_EQ(n_instruction_reads, 2);
}

TEST(if_injects_bubbles_while_memory_is_busy){
	sim_setup(0);		// no cache, dram only
	DRAM_READ_LATENCY = 3;
	uint32_t prog[] = {0x00c00093};
	sim_load_words(0, prog, 1);

	if_stage();
	CHECK_EQ(mfc_i, NOT_COMPLETED);
	CHECK_EQ(if_id.IR, 0);
	CHECK_STR_EQ(if_id.ins, BLANK_INS);
	CHECK_EQ(pc, 0);
	if_stage();
	CHECK_EQ(mfc_i, NOT_COMPLETED);
	if_stage();
	CHECK_EQ(mfc_i, COMPLETED);
	CHECK_EQ(if_id.IR, 0x00c00093);
	CHECK_EQ(pc, 4);
	CHECK_EQ(n_read_i_stalls, 2);
}

TEST(if_fetch_outside_text_segment_is_fatal){
	sim_setup(-1);
	pc = text_segment_limit + 1;
	CHECK_FATAL(if_stage(), "attempt to fetch instruction from non text segment");
}

TEST(if_unaligned_fetch_is_fatal){
	sim_setup(-1);
	pc = 2;
	CHECK_FATAL(if_stage(), "Unaligned instruction word access");
}

//------------------------------------------------------------------------------
// ID
//------------------------------------------------------------------------------

TEST(id_fills_id_ex){
	sim_setup(-1);
	reg_file[2] = 0x22;
	reg_file[3] = 0x33;
	sim_decode_into_id_ex(enc_r(0, 3, 2, 0, 1, OP_REG), 0x40);
	CHECK_EQ(id_ex.PC, 0x40);
	CHECK_EQ(id_ex.PC_next, 0x44);
	CHECK_EQ(id_ex.nrs1, 2);
	CHECK_EQ(id_ex.nrs2, 3);
	CHECK_EQ(id_ex.R1, 0x22);
	CHECK_EQ(id_ex.R2, 0x33);
	CHECK_EQ(id_ex.nrd, 1);
	CHECK_EQ(id_ex.imm, 0);
	CHECK(id_ex.ins == if_id.ins);

	sim_decode_into_id_ex(enc_i(-3, 2, 0, 5, OP_IMM), 0x40);
	CHECK_EQ(id_ex.imm, (uint32_t)-3);
	CHECK_EQ(id_ex.nrd, 5);
}

TEST(id_predicted_taken_branch_redirects_next_cycle){
	sim_setup(-1);
	BRANCH_PREDICTION_STRATEGY = TAKEN;
	pc = 0x108;	// IF already moved on
	sim_decode_into_id_ex(enc_b(0x40, 0, 0, 0), 0x100);
	CHECK_EQ(FLUSH, 0);
	CHECK_EQ(pc, 0x108);

	// next cycle: id_stage only performs the redirect
	if_id.IR = 0x00000013;
	id_ex.nrd = 99;
	id_stage();
	CHECK_EQ(FLUSH, 1);
	CHECK_EQ(pc, 0x140);
	CHECK_EQ(id_ex.nrd, 99);	// nothing was decoded this cycle
}

TEST(id_not_taken_prediction_does_not_redirect){
	sim_setup(-1);
	BRANCH_PREDICTION_STRATEGY = NOT_TAKEN;
	pc = 0x108;
	sim_decode_into_id_ex(enc_b(0x40, 0, 0, 0), 0x100);
	sim_decode_into_id_ex(0x00000013, 0x104);
	CHECK_EQ(FLUSH, 0);
	CHECK_EQ(pc, 0x108);
}

TEST(id_reset_decode_flags_cancels_redirect){
	sim_setup(-1);
	BRANCH_PREDICTION_STRATEGY = TAKEN;
	pc = 0x108;
	sim_decode_into_id_ex(enc_b(0x40, 0, 0, 0), 0x100);
	reset_decode_flags();		// e.g. an EX flush happened
	sim_decode_into_id_ex(0x00000013, 0x104);
	CHECK_EQ(FLUSH, 0);
	CHECK_EQ(pc, 0x108);
}

TEST(id_without_forwarding_sets_data_hazard_stall){
	sim_setup(-1);
	FORWARDING_SWITCH = 0;
	ex_ma.nrd = 2;
	ex_ma.cs_wb.wb = 1;
	sim_decode_into_id_ex(enc_r(0, 3, 2, 0, 1, OP_REG), 0x100);
	CHECK_EQ(DATA_HAZARD_STALL, 1);
}

TEST(id_with_forwarding_never_sets_data_hazard_stall){
	sim_setup(-1);
	FORWARDING_SWITCH = 1;
	ex_ma.nrd = 2;
	ex_ma.cs_wb.wb = 1;
	sim_decode_into_id_ex(enc_r(0, 3, 2, 0, 1, OP_REG), 0x100);
	CHECK_EQ(DATA_HAZARD_STALL, 0);
}

TEST(id_stalled_branch_does_not_arm_redirect){
	sim_setup(-1);
	FORWARDING_SWITCH = 0;
	BRANCH_PREDICTION_STRATEGY = TAKEN;
	ex_ma.nrd = 2;
	ex_ma.cs_wb.wb = 1;
	pc = 0x108;
	sim_decode_into_id_ex(enc_b(0x40, 3, 2, 0), 0x100);
	CHECK_EQ(DATA_HAZARD_STALL, 1);
	ex_ma.cs_wb.wb = 0;
	DATA_HAZARD_STALL = 0;
	sim_decode_into_id_ex(0x00000013, 0x104);
	CHECK_EQ(FLUSH, 0);
}

TEST(update_operands_rereads_register_file){
	sim_setup(-1);
	reg_file[2] = 1;
	reg_file[3] = 2;
	sim_decode_into_id_ex(enc_r(0, 3, 2, 0, 1, OP_REG), 0x100);
	reg_file[2] = 10;
	reg_file[3] = 20;
	update_operands();
	CHECK_EQ(id_ex.R1, 10);
	CHECK_EQ(id_ex.R2, 20);
}

//------------------------------------------------------------------------------
// MA
//------------------------------------------------------------------------------

static void ma_branch(uint8_t predicted, uint8_t actual){
	ex_ma.cs_ma.mem = 0;
	ex_ma.cs_ma.jump_or_branch = 2;
	ex_ma.cs_ma.branch_taken = predicted | (actual << 1);
	ex_ma.PC_next = 0x200;	// target
	ex_ma.result = 0x104;	// fall through
	ma_stage();
}

TEST(ma_correct_predictions_do_nothing){
	sim_setup(-1);
	pc = 0x50;
	ma_branch(0, 0);
	ma_branch(1, 1);
	CHECK_EQ(pc, 0x50);
	CHECK_EQ(FLUSH, 0);
	CHECK_EQ(branch_hit, 2);
	CHECK_EQ(branch_miss, 0);
	CHECK_EQ(branches_taken, 1);
	CHECK_EQ(branches_not_taken, 1);
	CHECK_EQ(n_branch_instructions, 2);
}

TEST(ma_mispredicted_taken_branch_redirects_to_target){
	sim_setup(-1);
	ma_branch(0, 1);
	CHECK_EQ(pc, 0x200);
	CHECK_EQ(FLUSH, 1);
	CHECK_EQ(branch_miss, 1);
	CHECK_EQ(branches_taken, 1);
}

TEST(ma_mispredicted_not_taken_branch_redirects_to_fall_through){
	sim_setup(-1);
	ma_branch(1, 0);
	CHECK_EQ(pc, 0x104);
	CHECK_EQ(FLUSH, 1);
	CHECK_EQ(branch_miss, 1);
	CHECK_EQ(branches_not_taken, 1);
}

TEST(ma_jump_always_redirects){
	sim_setup(-1);
	ex_ma.cs_ma.jump_or_branch = 1;
	ex_ma.PC_next = 0x300;
	ex_ma.result = 0x104;
	ex_ma.nrd = 1;
	ex_ma.cs_wb.wb = 1;
	ma_stage();
	CHECK_EQ(pc, 0x300);
	CHECK_EQ(FLUSH, 1);
	CHECK_EQ(ma_wb.result, 0x104);	// link value goes on to WB
	CHECK_EQ(n_branch_instructions, 0);
}

TEST(ma_passes_alu_result_through){
	sim_setup(-1);
	ex_ma.result = 0xABCD;
	ex_ma.nrd = 7;
	ex_ma.cs_wb.wb = 1;
	ex_ma.ins = "x                   ";
	ma_stage();
	CHECK_EQ(ma_wb.result, 0xABCD);
	CHECK_EQ(ma_wb.nrd, 7);
	CHECK_EQ(ma_wb.cs_wb.wb, 1);
	CHECK(ma_wb.ins == ex_ma.ins);
	CHECK_EQ(n_reads + n_writes, 0);
}

static uint32_t ma_load(uint32_t code, uint32_t addr){
	ex_ma.cs_ma.mem = 1;
	ex_ma.cs_ma.read_write_opcode = code;
	ex_ma.result = addr;
	ex_ma.cs_wb.wb = 1;
	ma_stage();
	return ma_wb.result;
}

TEST(ma_loads_with_sign_and_zero_extension){
	sim_setup(-1);
	sim_dram_set_word(0x40000, 0x8081F2F3);
	CHECK_EQ(ma_load(0b0010, 0x40000), 0x8081F2F3);	// lw
	CHECK_EQ(ma_load(0b0000, 0x40000), 0xFFFFFFF3);	// lb
	CHECK_EQ(ma_load(0b0011, 0x40000), 0xF3);		// lbu
	CHECK_EQ(ma_load(0b0000, 0x40003), 0xFFFFFF80);	// lb top byte
	CHECK_EQ(ma_load(0b0011, 0x40003), 0x80);
	CHECK_EQ(ma_load(0b0001, 0x40000), 0xFFFFF2F3);	// lh
	CHECK_EQ(ma_load(0b0100, 0x40000), 0xF2F3);		// lhu
	CHECK_EQ(ma_load(0b0001, 0x40002), 0xFFFF8081);
	CHECK_EQ(ma_load(0b0100, 0x40002), 0x8081);
	sim_dram_set_word(0x40004, 0x00007F7F);
	CHECK_EQ(ma_load(0b0000, 0x40004), 0x7F);		// positive byte
	CHECK_EQ(ma_load(0b0001, 0x40004), 0x7F7F);		// positive half
	CHECK_EQ(n_reads, 11);
}

static void ma_store(uint32_t code, uint32_t addr, uint32_t data){
	ex_ma.cs_ma.mem = 1;
	ex_ma.cs_ma.read_write_opcode = code;
	ex_ma.result = addr;
	ex_ma.R2 = data;
	ex_ma.nrs2 = 9;
	ex_ma.cs_wb.wb = 0;
	ma_stage();
}

TEST(ma_stores_bytes_halves_words_little_endian){
	sim_setup(-1);
	ma_store(0b1010, 0x40000, 0x11223344);
	CHECK_EQ(dram[0x40000], 0x44);
	CHECK_EQ(dram[0x40003], 0x11);
	ma_store(0b1000, 0x40001, 0xFFFFFFAA);	// sb only touches one byte
	CHECK_EQ(sim_dram_word(0x40000), 0x1122AA44);
	ma_store(0b1001, 0x40002, 0xFFFFBEEF);	// sh only touches two bytes
	CHECK_EQ(sim_dram_word(0x40000), 0xBEEFAA44);
	CHECK_EQ(n_writes, 3);
}

TEST(ma_store_data_forwarded_from_writeback){
	sim_setup(-1);
	ma_wb.nrd = 9;
	ma_wb.result = 0x5555;
	ma_wb.cs_wb.wb = 1;
	ma_store(0b1010, 0x40000, 0x1111);
	CHECK_EQ(sim_dram_word(0x40000), 0x5555);
	CHECK_EQ(n_forwards_to_ma, 1);
}

TEST(ma_illegal_accesses_are_fatal){
	sim_setup(-1);
	CHECK_FATAL(ma_store(0b1010, 0x100, 1), "attempt to write to text segment");
	CHECK_FATAL(ma_store(0b1010, text_segment_limit + 1, 1), "attempt to write to rodata segment");
	CHECK_FATAL(ma_store(0b1010, rodata_segment_limit - 3, 1), "attempt to write to rodata segment");
	CHECK_FATAL(ma_store(0b1010, 0x40002, 1), "Unaligned word access");
	CHECK_FATAL(ma_store(0b1001, 0x40001, 1), "Unaligned halfword access");
	CHECK_FATAL(ma_store(0b1010, DRAM_SIZE, 1), "Out of bounds memory write.");
	CHECK_FATAL(ma_load(0b0010, 0x40002), "Unaligned word access");
	CHECK_FATAL(ma_load(0b0001, 0x40001), "Unaligned halfword access");
	CHECK_FATAL(ma_load(0b0100, 0x40003), "Unaligned halfword access");
	CHECK_FATAL(ma_load(0b0010, DRAM_SIZE), "Out of bounds memory read.");
}

TEST(ma_store_to_first_data_address_is_allowed){
	sim_setup(-1);
	CHECK_SURVIVES(ma_store(0b1010, rodata_segment_limit + 1, 7));
	CHECK_SURVIVES(ma_store(0b1010, DRAM_SIZE - 4, 7));
}

TEST(ma_reading_text_and_rodata_is_allowed){
	sim_setup(-1);
	sim_dram_set_word(0x20000, 0x12345678);
	CHECK_EQ(ma_load(0b0010, 0x20000), 0x12345678);
	CHECK_SURVIVES(ma_load(0b0010, 0));
}

TEST(ma_load_waits_for_memory_latency){
	sim_setup(0);
	DRAM_READ_LATENCY = 4;
	sim_dram_set_word(0x40000, 77);
	ma_load(0b0010, 0x40000);
	CHECK_EQ(mfc, NOT_COMPLETED);
	ma_load(0b0010, 0x40000);
	ma_load(0b0010, 0x40000);
	CHECK_EQ(mfc, NOT_COMPLETED);
	CHECK_EQ(ma_load(0b0010, 0x40000), 77);
	CHECK_EQ(mfc, COMPLETED);
	CHECK_EQ(n_read_d_stalls, 3);
}

//------------------------------------------------------------------------------
// WB
//------------------------------------------------------------------------------

TEST(wb_writes_register_file){
	sim_setup(-1);
	ma_wb.nrd = 5;
	ma_wb.result = 0x99;
	ma_wb.cs_wb.wb = 1;
	ma_wb.ins = "addi                ";
	wb_stage();
	CHECK_EQ(reg_file[5], 0x99);
	CHECK_EQ(wb_if.nrd, 5);
	CHECK_EQ(wb_if.result, 0x99);
	CHECK_EQ(wb_if.cs_wb.wb, 1);
	CHECK_EQ(n_instructions, 1);
}

TEST(wb_keeps_x0_zero){
	sim_setup(-1);
	ma_wb.nrd = 0;
	ma_wb.result = 0x99;
	ma_wb.cs_wb.wb = 1;
	wb_stage();
	CHECK_EQ(reg_file[0], 0);
}

TEST(wb_idle_does_not_write){
	sim_setup(-1);
	reg_file[5] = 1;
	ma_wb.nrd = 5;
	ma_wb.result = 0x99;
	ma_wb.cs_wb.wb = 0;
	wb_stage();
	CHECK_EQ(reg_file[5], 1);
}

TEST(wb_keeps_history_for_forwarding){
	sim_setup(-1);
	ma_wb = (MA_WB){0x11, 3, {1}, "first               "};
	wb_stage();
	ma_wb = (MA_WB){0x22, 4, {1}, "second              "};
	wb_stage();
	CHECK_EQ(recent_wb.result, 0x11);
	CHECK_EQ(recent_wb.nrd, 3);
	CHECK_EQ(wb_if.result, 0x22);
	CHECK_EQ(wb_if.nrd, 4);
}

TEST(wb_counts_only_real_instructions){
	sim_setup(-1);
	ma_wb.ins = BLANK_INS;		// bubble
	wb_stage();
	ma_wb.ins = "sw r1, 0(r2)        ";	// retires even without a write back
	wb_stage();
	CHECK_EQ(n_instructions, 1);
}

TEST_SUITE("stages",
	T(if_fetches_and_advances_pc),
	T(if_injects_bubbles_while_memory_is_busy),
	T(if_fetch_outside_text_segment_is_fatal),
	T(if_unaligned_fetch_is_fatal),
	T(id_fills_id_ex),
	T(id_predicted_taken_branch_redirects_next_cycle),
	T(id_not_taken_prediction_does_not_redirect),
	T(id_reset_decode_flags_cancels_redirect),
	T(id_without_forwarding_sets_data_hazard_stall),
	T(id_with_forwarding_never_sets_data_hazard_stall),
	T(id_stalled_branch_does_not_arm_redirect),
	T(update_operands_rereads_register_file),
	T(ma_correct_predictions_do_nothing),
	T(ma_mispredicted_taken_branch_redirects_to_target),
	T(ma_mispredicted_not_taken_branch_redirects_to_fall_through),
	T(ma_jump_always_redirects),
	T(ma_passes_alu_result_through),
	T(ma_loads_with_sign_and_zero_extension),
	T(ma_stores_bytes_halves_words_little_endian),
	T(ma_store_data_forwarded_from_writeback),
	T(ma_illegal_accesses_are_fatal),
	T(ma_store_to_first_data_address_is_allowed),
	T(ma_reading_text_and_rodata_is_allowed),
	T(ma_load_waits_for_memory_latency),
	T(wb_writes_register_file),
	T(wb_keeps_x0_zero),
	T(wb_idle_does_not_write),
	T(wb_keeps_history_for_forwarding),
	T(wb_counts_only_real_instructions),
)
