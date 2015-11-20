#pragma scop
/* Clay
   embed([0]);
 */
S1;
for (int i = 0; i < N; i++)
  S2(i);
#pragma endscop
