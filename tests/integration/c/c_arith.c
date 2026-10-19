// integer arithmetic: software mul/div/mod (essentials/stdlib), shifts,
// comparisons, char/short sign extension through memory
#include "c_test.h"

static volatile int32_t sv[] = {7, -7, 100, -100, 12345, -98765, 1, -1, 2147483647, 3};
static volatile uint32_t uv[] = {7, 0xFFFFFFF9u, 100, 0x80000000u, 12345, 4000000000u, 1, 0xFFFFFFFFu, 65536, 3};
static volatile int8_t bytes[] = {-128, -1, 0, 1, 127, -77};
static volatile uint8_t ubytes[] = {255, 128, 0, 1, 127, 179};
static volatile int16_t halves[] = {-32768, -1, 0, 1, 32767, -1234};
static volatile uint16_t uhalves[] = {65535, 32768, 0, 1, 32767, 64302};

uint32_t test_main(void){
	uint32_t h = 2166136261u;
	for(int i = 0; i < 10; i++){
		for(int j = 0; j < 10; j++){
			int32_t a = sv[i], b = sv[j];
			uint32_t ua = uv[i], ub = uv[j];
			h = mix(h, (uint32_t)a * (uint32_t)b);
			h = mix(h, ua * ub);
			if(b != 0 && !(a == (-2147483647 - 1) && b == -1)){
				h = mix(h, (uint32_t)(a / b));
				h = mix(h, (uint32_t)(a % b));
			}
			if(ub != 0){
				h = mix(h, ua / ub);
				h = mix(h, ua % ub);
			}
			h = mix(h, (uint32_t)(a < b) | ((uint32_t)(ua < ub) << 1) | ((uint32_t)(a >= b) << 2));
			h = mix(h, ua << (j * 3 % 32));
			h = mix(h, ua >> (j * 3 % 32));
			h = mix(h, (uint32_t)(a >> (j * 3 % 32)));
		}
	}
	for(int i = 0; i < 6; i++){
		h = mix(h, (uint32_t)(int32_t)bytes[i]);
		h = mix(h, (uint32_t)ubytes[i]);
		h = mix(h, (uint32_t)(int32_t)halves[i]);
		h = mix(h, (uint32_t)uhalves[i]);
		bytes[i] = (int8_t)(bytes[i] + 3);
		halves[i] = (int16_t)(halves[i] * 3);
		h = mix(h, (uint32_t)(int32_t)bytes[i] + (uint32_t)(int32_t)halves[i]);
	}
	return h;
}
