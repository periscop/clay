#pragma scop
/* Clay
   peel([0], {-1 | -25});
*/
for(i = 10 ; i <= N-8 ; i++) {
  a[i] = 0;
}
#pragma endscop
