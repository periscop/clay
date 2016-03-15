#pragma scop
/* Clay
   stripmine([0],1,1);
   linearize([0],1);
#   skew([0],1,2,1);
#   reshape([0],1,1,-1);
#   interchange([0,0],1,2,0);
 */
for (int i = 0; i < N; i++) {
  S2(i);
}
#pragma endscop
