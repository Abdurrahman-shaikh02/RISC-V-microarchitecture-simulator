// Every C test program defines test_main() and returns a 32 bit checksum.
// The same source is compiled for the simulator (result in a0 = r10) and for
// the host (result printed), and the two results must be identical.
#include <stdint.h>

uint32_t test_main(void);

#ifdef HOST
#include <stdio.h>
int main(void){ printf("%u\n", test_main()); return 0; }
#else
int main(void){ return (int)test_main(); }
#endif

static uint32_t mix(uint32_t h, uint32_t v){
	return (h ^ v) * 16777619u + (h >> 7);
}
