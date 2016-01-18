#pragma scop
/* Clay
   iss([0], {1||-3});
   collapse([0]);
 */
for (i = 0; i < N; i++) {
  S(i);
  S2(i);
}
#pragma endscop
