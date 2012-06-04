#pragma scop
/* Clay
   reverse([0,1,1], 2);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
  for(j = 0 ; j <= M ; j++) {
    b[i][j] = 0;
    c[i][j] = 0;
    d[i][j] = 0;
  }
}
#pragma endscop
