#pragma scop
/* Clay
   split([0,1], 1);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
  for(j = 0 ; j <= M ; j++) {
    b[i][j] = 0;
    c[i][j] = 0;
  }
  d[i] = 0;
  e[i] = 0;
}
f = 0;
#pragma endscop
