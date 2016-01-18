#pragma scop
/* Clay
   grain([0,0],1,3);
   densify([0,0],1);
 */
for (int i = 0; i < N; i++) {
  S1(i);
  S2(i);
}
#pragma endscop
