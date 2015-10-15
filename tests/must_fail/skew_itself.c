#pragma scop
/* Clay
   skew([0,0],1,1,1);
*/
for (i = 0; i < N; i++) {
  for (j = 0; j < M; j++) {
    S(i, j);
  }
}
#pragma endscop
