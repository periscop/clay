#pragma scop
/* Clay
   stripmine([1],1,4);
   linearize([1],1);
 */
for (int i = 0; i < N; i++) {
  S1(i);
  S2(i);
}
for (int i = 0; i < N; i++) {
  S1(i);
  S2(i);
}

#pragma endscop
