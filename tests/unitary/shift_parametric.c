#pragma scop
/* Clay
   shift([0],1,[-1,1],0);
 */
for (i = M; i < N; i++) {
  S(i);
}
#pragma endscop
