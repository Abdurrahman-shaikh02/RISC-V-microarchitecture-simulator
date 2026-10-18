#include "common.h"
#define N 1024
u32 a[N], b[N]; u32 cnt[256];
int main(void){ for(int i=0;i<N;i++) a[i]=lcg();
  u32 *src=a,*dst=b;
  for(int sh=0; sh<32; sh+=8){
    for(int i=0;i<256;i++) cnt[i]=0;
    for(int i=0;i<N;i++) cnt[(src[i]>>sh)&255]++;
    u32 s=0; for(int i=0;i<256;i++){ u32 c=cnt[i]; cnt[i]=s; s+=c; }
    for(int i=0;i<N;i++) dst[cnt[(src[i]>>sh)&255]++]=src[i];
    u32 *t=src; src=dst; dst=t; }
  for(int i=1;i<N;i++) if(src[i-1]>src[i]) return -1; return (int)hash_arr(src,N); }
