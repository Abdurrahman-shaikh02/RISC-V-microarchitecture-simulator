#include "common.h"
#define N 1024
u32 a[N];
static void qs(u32 *v, int lo, int hi){
  while(lo < hi){
    u32 p = v[(lo+hi)>>1]; int i = lo, j = hi;
    while(i <= j){ while(v[i] < p) i++; while(v[j] > p) j--; if(i <= j){ u32 t=v[i]; v[i]=v[j]; v[j]=t; i++; j--; } }
    if(j - lo < hi - i){ qs(v, lo, j); lo = i; } else { qs(v, i, hi); hi = j; }
  }
}
int main(void){ for(int i=0;i<N;i++) a[i]=lcg(); qs(a,0,N-1);
  for(int i=1;i<N;i++) if(a[i-1]>a[i]) return -1; return (int)hash_arr(a,N); }
