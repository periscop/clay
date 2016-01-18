#pragma scop
/* Clay
   grain([0,0],2,4);
   densify([0,0],2);
 */
for (int i = 0; i < N; i++) {
  for (int j = 0; j < N; j+=2) {
    S1(i);
    S2(i);
  }
}
#pragma endscop
