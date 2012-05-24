#pragma scop
/* Clay
   peel([0], [0,5], 0);
*/
for(i = 10 ; i <= N-8 ; i++) {
  a[i] = 0;
}
#pragma endscop
