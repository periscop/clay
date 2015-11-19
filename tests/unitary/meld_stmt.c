#pragma scop
/* Clay
   meld([0]);
 */
S1;
for (int i = 0; i < N; i++)
  S2(i);
#pragma endscop
