// pseudo random data sorted three ways (insertion, recursive quicksort,
// counting sort); returns a checksum, or 0xBAD if any result is not sorted
#include "c_test.h"

#define N 300
static uint32_t a[N], b[N], c[N];
static uint32_t counts[256];

static void insertion(uint32_t * v, int n){
	for(int i = 1; i < n; i++){
		uint32_t x = v[i];
		int j = i - 1;
		while(j >= 0 && v[j] > x){
			v[j + 1] = v[j];
			j--;
		}
		v[j + 1] = x;
	}
}

static void quick(uint32_t * v, int lo, int hi){
	if(lo >= hi) return;
	uint32_t p = v[(lo + hi) / 2];
	int i = lo, j = hi;
	while(i <= j){
		while(v[i] < p) i++;
		while(v[j] > p) j--;
		if(i <= j){
			uint32_t t = v[i]; v[i] = v[j]; v[j] = t;
			i++; j--;
		}
	}
	quick(v, lo, j);
	quick(v, i, hi);
}

uint32_t test_main(void){
	uint32_t seed = 12345;
	for(int i = 0; i < N; i++){
		seed = seed * 1103515245u + 12345u;
		a[i] = b[i] = (seed >> 8) & 0xFFFF;
		c[i] = (seed >> 16) & 0xFF;
	}
	insertion(a, N);
	quick(b, 0, N - 1);
	for(int i = 0; i < N; i++) counts[c[i]]++;
	int k = 0;
	for(int v = 0; v < 256; v++)
		for(uint32_t n = 0; n < counts[v]; n++) c[k++] = (uint32_t)v;

	uint32_t h = 2166136261u;
	for(int i = 0; i < N; i++){
		if(a[i] != b[i]) return 0xBAD;
		if(i > 0 && (a[i - 1] > a[i] || c[i - 1] > c[i])) return 0xBAD;
		h = mix(h, a[i] + (c[i] << 16));
	}
	return h;
}
