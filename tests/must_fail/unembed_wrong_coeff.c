#pragma scop
/* Clay
   unembed([0,1,0]);
 */
for (int t = 0; t < T; t++) {
  S1(t);
  for (int i = 0; i < N; i++) {
    S3(t,i);
  }
  S4(t);
}
for (int t = 0; t < T; t++) {
  S5(t);
}
#pragma endscop
