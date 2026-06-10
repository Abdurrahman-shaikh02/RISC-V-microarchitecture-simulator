#include "header.h"
#include "memory.h"
#include "cache.h"

void display_valid(cache_line * cache){
	for(int i = 0; i < 4096; i++){
		printf("%d\n", cache[i].valid);
	}
}

int main(){
	cache_line * cache = (cache_line *)malloc(4096 * sizeof(cache_line));

	for(int i = 0; i < 4096; i++){
		cache[i].valid = 0;
		cache[i].data = 0;
		cache[i].dirty = 0;
		cache[i].done = 0;
		cache[i].tag = 0;
	}

	for(int i = 0; i < 16; i++){
		cache[i].tag = 0x0004FFFF;
		cache[i].valid = 1;
		cache[i].done = 0;
	}
	for(int i = 16; i < 32; i++){
		cache[i].tag = 0x0003FFFF;
		cache[i].valid = 1;
		cache[i].done = 0;
	}

	set_valid(0xFFFFE000, 1, cache, 4096*4, 64, 2);
	set_done(0xFFFFE000, 1, cache, 4096*4, 64, 2);
	int t = find_block(0xFFFFE000, cache, 4096*4, 64, 2);
	display_valid(cache);
	return t;

}
