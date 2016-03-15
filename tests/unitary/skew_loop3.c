#pragma scop
/* Clay
   skew([0,0],2,1,-1);
 */
for (i = 0; i <= N; i++) {
  for (j = 0; j <= M; j++) {
    A[i][j] = 0;
  }
  B[i] = 0;
}
#pragma endscop
