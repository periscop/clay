#pragma scop
/* Clay
   skew([0,0,0], 3, 3, 1);
*/
for(i = 0 ; i <= N ; i++) {
  for(j = 0 ; j <= M ; j++) {
    a[i][j] = 0;
  }
}
#pragma endscop
