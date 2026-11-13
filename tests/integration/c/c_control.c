// control flow and memory layout: switch jump tables (rodata), function
// pointers (jalr), strings, initialised globals (.data), zeroed globals
// (.bss), struct copies, deep recursion
#include "c_test.h"

typedef struct { int32_t x, y; uint8_t tag; int16_t w; } point;

static const char message[] = "RISC-V pipeline simulator";
static int32_t table[8] = {3, -1, 4, -1, 5, -9, 2, 6};
static uint32_t zeroed[64];
static point origin = {10, -20, 7, -300};

static uint32_t op_add(uint32_t a, uint32_t b){ return a + b; }
static uint32_t op_xor(uint32_t a, uint32_t b){ return a ^ b; }
static uint32_t op_sub(uint32_t a, uint32_t b){ return a - b; }
static uint32_t (*const ops[3])(uint32_t, uint32_t) = {op_add, op_xor, op_sub};

static __attribute__((noinline)) uint32_t classify(uint32_t v){
	switch(v % 9){
		case 0: return 11;
		case 1: return 23;
		case 2: return 5;
		case 3: return 99;
		case 4: return 1000;
		case 5: return 7;
		case 6: return 64;
		case 7: return 3;
		default: return 42;
	}
}

static __attribute__((noinline)) uint32_t depth(uint32_t n){
	return n == 0 ? 1 : 3 * depth(n - 1) + n;
}

static __attribute__((noinline)) point move(point p, int32_t dx){
	p.x += dx;
	p.y -= dx;
	p.tag++;
	p.w = (int16_t)(p.w + dx);
	return p;
}

uint32_t test_main(void){
	uint32_t h = 2166136261u;
	for(uint32_t i = 0; message[i]; i++) h = mix(h, (uint32_t)message[i]);
	for(int i = 0; i < 8; i++) h = mix(h, (uint32_t)table[i]);
	for(int i = 0; i < 64; i++) h = mix(h, zeroed[i] + 1);
	for(uint32_t i = 0; i < 40; i++) h = mix(h, classify(i * 7 + 3));
	for(uint32_t i = 0; i < 30; i++) h = ops[i % 3](h, i * 2654435761u);
	h = mix(h, depth(200));
	point p = origin;
	for(int i = 0; i < 5; i++) p = move(p, i - 2);
	h = mix(h, (uint32_t)p.x);
	h = mix(h, (uint32_t)p.y);
	h = mix(h, p.tag);
	h = mix(h, (uint32_t)(int32_t)p.w);
	table[3] = (int32_t)h;		// globals are writable
	zeroed[5] = h;
	h = mix(h, (uint32_t)table[3] ^ zeroed[5]);
	return h;
}
