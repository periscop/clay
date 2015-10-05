#pragma scop
/* Clay
   iss([0,2], {1||-3});
   collapse([0,2]);
 */
for (i = 0; i < N; i++) {
  S(i);
  S2(i);
  for (j = 0; j < P; j++) {
    S4(i, j);
    S5(j, i);
  }
  S6(i);
}
#pragma endscop
