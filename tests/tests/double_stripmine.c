#pragma scop
/* Clay
  stripmine([0],1,4,0);
  stripmine([0,0],2,3,0);
#  linearize([0,0],2);
#  linearize([0],1);
 */
for (i = 0; i < N; i++) {
  S(i);
}
#pragma endscop