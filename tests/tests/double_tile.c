#pragma scop
/* Clay
   tile([0,0,0],1,1,4,0);
   tile([0,0,0,0],3,3,4,0);
 */
for (i=0;i<=N-1;i++) {
  for (j=0;j<=N-1;j++) {
    Z[i + j] += 1;
  }
}
#pragma endscop
