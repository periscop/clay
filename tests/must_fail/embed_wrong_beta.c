#pragma scop
/* Clay
   embed([0]);
   fuse([0]);
   unembed([0,0]);
 */
S1;
for (i = 0; i < N; i++)
  S2(i);
#pragma endscop
