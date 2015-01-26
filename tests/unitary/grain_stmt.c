#pragma scop
/* Clay
   grain([0,0],1,3);
 */
for (int i = 0; i < N; i++) {
  S(i);
  S2(i);
}
#pragma endscop
