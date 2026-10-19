typedef unsigned int u32;
static u32 lcg_state = 12345u;
static u32 lcg(void){ lcg_state = lcg_state * 1103515245u + 12345u; return lcg_state >> 1; }
static u32 hash_arr(const u32 *a, int n){ u32 h = 2166136261u; for(int i=0;i<n;i++){ h ^= a[i]; h = (h << 5) | (h >> 27); h += 0x9e3779b9u; } return h; }
