#pragma scop
/* Clay
   skew([1,1,1], 1, 1);
*/
z = 0;
for(i = 0 ; i <= N ; i++) {
  y[i] = 0;
  for(j = 0 ; j <= M ; j++) {
    a[i][j] = 0;
    b[i][j] = 0;
    c[i][j] = 0;
  }
}
#pragma endscop
