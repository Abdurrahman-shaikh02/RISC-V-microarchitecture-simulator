// src/forwarding/forwarding.c (forward_ex, forward_ma) and the stall based
// hazard detection used when forwarding is off (set_hazard_stall in id.c)
#include "sim_support.h"

int set_hazard_stall();	// not declared in any header (src/pipeline/id.c)

static void producer_in_ex_ma(uint32_t rd, uint32_t value, int is_load){
	ex_ma.nrd = rd;
	ex_ma.result = value;
	ex_ma.cs_wb.wb = 1;
	ex_ma.cs_ma.mem = is_load;
	ex_ma.cs_ma.read_write_opcode = is_load ? 0b0010 : 0;
}

static void producer_in_ma_wb(uint32_t rd, uint32_t value){
	ma_wb.nrd = rd;
	ma_wb.result = value;
	ma_wb.cs_wb.wb = 1;
}

static void producer_in_wb_if(uint32_t rd, uint32_t value){
	wb_if.nrd = rd;
	wb_if.result = value;
	wb_if.cs_wb.wb = 1;
}

//------------------------------------------------------------------------------
// forward_ex
//------------------------------------------------------------------------------

TEST(ex_no_hazard_returns_original){
	sim_setup(-1);
	CHECK_EQ(forward_ex(0x1234, 5), 0x1234);
	CHECK_EQ(FORWARDING_STALL, 0);
	CHECK_EQ(n_forwards_to_ex, 0);
}

TEST(ex_forwards_alu_result_from_ex_ma){
	sim_setup(-1);
	producer_in_ex_ma(5, 0xAAAA, 0);
	CHECK_EQ(forward_ex(0x1234, 5), 0xAAAA);
	CHECK_EQ(FORWARDING_STALL, 0);
	CHECK_EQ(n_forwards_to_ex, 1);
}

TEST(ex_load_in_ex_ma_forces_stall){
	sim_setup(-1);
	producer_in_ex_ma(5, 0xAAAA, 1);
	forward_ex(0x1234, 5);
	CHECK_EQ(FORWARDING_STALL, 1);
	CHECK_EQ(n_forwards_to_ex, 0);
}

TEST(ex_value_in_ma_wb_forces_stall){
	// ma happens before ex in the same cycle, so a value that just reached
	// ma_wb was produced by a memory access this cycle: must stall
	sim_setup(-1);
	producer_in_ma_wb(5, 0xBBBB);
	forward_ex(0x1234, 5);
	CHECK_EQ(FORWARDING_STALL, 1);
}

TEST(ex_forwards_from_wb_if){
	sim_setup(-1);
	producer_in_wb_if(5, 0xCCCC);
	CHECK_EQ(forward_ex(0x1234, 5), 0xCCCC);
	CHECK_EQ(FORWARDING_STALL, 0);
	CHECK_EQ(n_forwards_to_ex, 1);
}

TEST(ex_youngest_producer_wins){
	sim_setup(-1);
	producer_in_ex_ma(5, 0xAAAA, 0);
	producer_in_wb_if(5, 0xCCCC);
	CHECK_EQ(forward_ex(0x1234, 5), 0xAAAA);
}

TEST(ex_never_forwards_x0){
	sim_setup(-1);
	producer_in_ex_ma(0, 0xAAAA, 0);
	producer_in_ma_wb(0, 0xBBBB);
	producer_in_wb_if(0, 0xCCCC);
	CHECK_EQ(forward_ex(0, 0), 0);
	CHECK_EQ(FORWARDING_STALL, 0);
}

TEST(ex_ignores_producers_without_writeback){
	sim_setup(-1);
	producer_in_ex_ma(5, 0xAAAA, 0);
	ex_ma.cs_wb.wb = 0;			// e.g. a store or a branch
	producer_in_ma_wb(5, 0xBBBB);
	ma_wb.cs_wb.wb = 0;
	CHECK_EQ(forward_ex(0x1234, 5), 0x1234);
	CHECK_EQ(FORWARDING_STALL, 0);
}

TEST(ex_other_registers_are_not_forwarded){
	sim_setup(-1);
	producer_in_ex_ma(6, 0xAAAA, 0);
	CHECK_EQ(forward_ex(0x1234, 5), 0x1234);
}

//------------------------------------------------------------------------------
// forward_ma (store data)
//------------------------------------------------------------------------------

TEST(ma_no_hazard_returns_original){
	sim_setup(-1);
	CHECK_EQ(forward_ma(0x1234, 7), 0x1234);
	CHECK_EQ(n_forwards_to_ma, 0);
}

TEST(ma_forwards_from_ma_wb){
	sim_setup(-1);
	producer_in_ma_wb(7, 0xBBBB);
	CHECK_EQ(forward_ma(0x1234, 7), 0xBBBB);
	CHECK_EQ(n_forwards_to_ma, 1);
}

TEST(ma_forwards_from_recent_wb){
	sim_setup(-1);
	recent_wb.nrd = 7;
	recent_wb.result = 0xDDDD;
	recent_wb.cs_wb.wb = 1;
	CHECK_EQ(forward_ma(0x1234, 7), 0xDDDD);
	CHECK_EQ(n_forwards_to_ma, 1);
}

TEST(ma_prefers_ma_wb_over_recent_wb){
	sim_setup(-1);
	producer_in_ma_wb(7, 0xBBBB);
	recent_wb.nrd = 7;
	recent_wb.result = 0xDDDD;
	recent_wb.cs_wb.wb = 1;
	CHECK_EQ(forward_ma(0x1234, 7), 0xBBBB);
}

TEST(ma_never_forwards_x0){
	sim_setup(-1);
	producer_in_ma_wb(0, 0xBBBB);
	CHECK_EQ(forward_ma(0, 0), 0);
}

//------------------------------------------------------------------------------
// set_hazard_stall (forwarding off)
//------------------------------------------------------------------------------

static void consumer_add_r5_r6(void){
	id_ex.nrs1 = 5;
	id_ex.nrs2 = 6;
	id_ex.cs_ex.source1 = 1;
	id_ex.cs_ex.source2 = 0;
}

TEST(hazard_none){
	sim_setup(-1);
	consumer_add_r5_r6();
	producer_in_ex_ma(7, 0, 0);
	CHECK_EQ(set_hazard_stall(), 0);
	CHECK_EQ(DATA_HAZARD_STALL, 0);
}

TEST(hazard_on_rs1_and_rs2_in_every_later_stage){
	void (*producers[])(uint32_t, uint32_t) = {producer_in_ma_wb, producer_in_wb_if};
	for(int reg = 5; reg <= 6; reg++){
		sim_setup(-1);
		consumer_add_r5_r6();
		producer_in_ex_ma(reg, 0, 0);
		CHECK_EQ(set_hazard_stall(), 1);
		CHECK_EQ(DATA_HAZARD_STALL, 1);
		for(int p = 0; p < 2; p++){
			sim_setup(-1);
			consumer_add_r5_r6();
			producers[p](reg, 0);
			CHECK_MSG(set_hazard_stall() == 1, "no stall for r%d produced in stage %d", reg, p);
		}
	}
}

TEST(hazard_ignores_immediate_operand_register_field){
	// addi: the rs2 field holds immediate bits, it must not cause a stall
	sim_setup(-1);
	id_ex.nrs1 = 1;
	id_ex.nrs2 = 6;
	id_ex.cs_ex.source1 = 1;
	id_ex.cs_ex.source2 = 1;
	producer_in_ex_ma(6, 0, 0);
	CHECK_EQ(set_hazard_stall(), 0);
}

TEST(hazard_on_store_data_register){
	sim_setup(-1);
	id_ex.nrs1 = 1;
	id_ex.nrs2 = 6;
	id_ex.cs_ex.source1 = 1;
	id_ex.cs_ex.source2 = 1;
	id_ex.cs_ma.mem = 1;
	id_ex.cs_ma.read_write_opcode = 0b1010;	// sw
	producer_in_ma_wb(6, 0);
	CHECK_EQ(set_hazard_stall(), 1);
}

TEST(hazard_never_on_x0){
	sim_setup(-1);
	id_ex.nrs1 = 0;
	id_ex.nrs2 = 0;
	id_ex.cs_ex.source1 = 1;
	id_ex.cs_ex.source2 = 0;
	producer_in_ex_ma(0, 0, 0);
	producer_in_ma_wb(0, 0);
	producer_in_wb_if(0, 0);
	CHECK_EQ(set_hazard_stall(), 0);
}

TEST_SUITE("forwarding",
	T(ex_no_hazard_returns_original),
	T(ex_forwards_alu_result_from_ex_ma),
	T(ex_load_in_ex_ma_forces_stall),
	T(ex_value_in_ma_wb_forces_stall),
	T(ex_forwards_from_wb_if),
	T(ex_youngest_producer_wins),
	T(ex_never_forwards_x0),
	T(ex_ignores_producers_without_writeback),
	T(ex_other_registers_are_not_forwarded),
	T(ma_no_hazard_returns_original),
	T(ma_forwards_from_ma_wb),
	T(ma_forwards_from_recent_wb),
	T(ma_prefers_ma_wb_over_recent_wb),
	T(ma_never_forwards_x0),
	T(hazard_none),
	T(hazard_on_rs1_and_rs2_in_every_later_stage),
	T(hazard_ignores_immediate_operand_register_field),
	T(hazard_on_store_data_register),
	T(hazard_never_on_x0),
)
