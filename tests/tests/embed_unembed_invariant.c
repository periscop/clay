#pragma scop
/* Clay
   embed([0]);
   embed([0,0]);
   embed([1,0]);
   embed([1,1,0]);
   unembed([0,0,0]);
   unembed([0,0]);
   unembed([1,0,0]);
   unembed([1,1,0,0]);
 */
S1;
for (i = 0; i < N; i++) {
  S2(i);
  for (j = 0; j < M; j++) {
    S3(i, j);
  }
}
#pragma endscop
