#pragma scop
/* Clay
   embed([0,0]);
 */
for (int t = 0; t < T; t++) {
  S1(t);
  for (int i = 0; i < N; i++) {
    S2(t,i);
    S3(t,i);
  }
  S4(t);
}
#pragma endscop
