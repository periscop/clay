#pragma scop
/* Clay
   reshape([0,0],2,1,1);
   reshape([0,0],1,2,2);
 */
for (i = 0; i < N; i++) {
  for (j = 0; j < M; j++) {
    S1(i, j);
    S2(i, j);
  }
}
#pragma endscop
