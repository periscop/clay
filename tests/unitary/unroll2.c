#pragma scop
/* Clay
   unroll([1,1,0], 4);
*/
a = 0;
for(i = 0 ; i <= N ; i++) {
  b[i] = 0;
  for(k = 0 ; k <= P ; k++) {
    for(j = 0 ; j <= M ; j++) {
      c[i][j][k] = 0;
      d[i][j][k] = 0;
    }
  }
  e[i] = 0;
}
f = 0;
#pragma endscop
