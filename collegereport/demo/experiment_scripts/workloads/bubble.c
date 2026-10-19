#include "common.h"
#define N 256
u32 a[N];
int main(void){ for(int i=0;i<N;i++) a[i]=lcg()&0xffff;
  for(int i=0;i<N-1;i++) for(int j=0;j<N-1-i;j++) if(a[j]>a[j+1]){ u32 t=a[j]; a[j]=a[j+1]; a[j+1]=t; }
  return (int)hash_arr(a,N); }
