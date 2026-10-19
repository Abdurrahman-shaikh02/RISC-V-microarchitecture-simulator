// src/branch/*: branch predictors and the predict()/store() dispatch
#include "sim_support.h"

TEST(default_strategy_is_not_taken){
	CHECK_EQ(BRANCH_PREDICTION_STRATEGY, NOT_TAKEN);
}

TEST(static_not_taken){
	CHECK_EQ(predict_not_taken(enc_b(-8, 1, 2, 0)), 0);
	CHECK_EQ(predict_not_taken(enc_b(8, 1, 2, 0)), 0);
}

TEST(static_taken){
	CHECK_EQ(predict_taken(enc_b(-8, 1, 2, 0)), 1);
	CHECK_EQ(predict_taken(enc_b(8, 1, 2, 0)), 1);
}

TEST(static_btfn_backward_taken_forward_not_taken){
	sim_quiet();
	CHECK_EQ(predict_based_on_offset(enc_b(-8, 1, 2, 0)), 1);
	CHECK_EQ(predict_based_on_offset(enc_b(-4096, 1, 2, 0)), 1);
	CHECK_EQ(predict_based_on_offset(enc_b(-2, 1, 2, 0)), 1);
	CHECK_EQ(predict_based_on_offset(enc_b(0, 1, 2, 0)), 0);
	CHECK_EQ(predict_based_on_offset(enc_b(8, 1, 2, 0)), 0);
	CHECK_EQ(predict_based_on_offset(enc_b(4094, 1, 2, 0)), 0);
}

TEST(predict_dispatches_on_strategy){
	sim_quiet();
	uint32_t backward = enc_b(-8, 1, 2, 0), forward = enc_b(8, 1, 2, 0);

	BRANCH_PREDICTION_STRATEGY = NOT_TAKEN;
	CHECK_EQ(predict(backward), 0);
	CHECK_EQ(predict(forward), 0);

	BRANCH_PREDICTION_STRATEGY = TAKEN;
	CHECK_EQ(predict(backward), 1);
	CHECK_EQ(predict(forward), 1);

	BRANCH_PREDICTION_STRATEGY = OFFSET;
	CHECK_EQ(predict(backward), 1);
	CHECK_EQ(predict(forward), 0);
}

TEST(dynamic_predictors_are_stubs_predicting_not_taken){
	// TWO_BIT_SWITCH and HISTORY_TABLE are declared but not implemented yet,
	// they currently behave like NOT_TAKEN. Update this test when they are.
	sim_quiet();
	BRANCH_PREDICTION_STRATEGY = TWO_BIT_SWITCH;
	store(1); store(1); store(1);
	CHECK_EQ(predict(enc_b(-8, 1, 2, 0)), 0);
	BRANCH_PREDICTION_STRATEGY = HISTORY_TABLE;
	store(1); store(1); store(1);
	CHECK_EQ(predict(enc_b(-8, 1, 2, 0)), 0);
}

TEST(store_is_harmless_for_static_predictors){
	sim_quiet();
	for(int s = NOT_TAKEN; s <= OFFSET; s++){
		BRANCH_PREDICTION_STRATEGY = s;
		CHECK_SURVIVES(store(1); store(0));
	}
}

TEST(invalid_strategy_is_fatal){
	sim_quiet();
	BRANCH_PREDICTION_STRATEGY = 7;
	CHECK_FATAL(predict(enc_b(8, 1, 2, 0)), "Invalid branch prediction strategy.");
}

TEST_SUITE("branch_prediction",
	T(default_strategy_is_not_taken),
	T(static_not_taken),
	T(static_taken),
	T(static_btfn_backward_taken_forward_not_taken),
	T(predict_dispatches_on_strategy),
	T(dynamic_predictors_are_stubs_predicting_not_taken),
	T(store_is_harmless_for_static_predictors),
	T(invalid_strategy_is_fatal),
)
