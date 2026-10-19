#include "common.h"
#define M 24
int A[M][M], B[M][M], C[M][M];
int main(void){ for(int i=0;i<M;i++) for(int j=0;j<M;j++){ A[i][j]=(int)(lcg()&255)-128; B[i][j]=(int)(lcg()&255)-128; }
  for(int i=0;i<M;i++) for(int j=0;j<M;j++){ int s=0; for(int k=0;k<M;k++) s+=A[i][k]*B[k][j]; C[i][j]=s; }
  return (int)hash_arr((u32*)C, M*M); }
