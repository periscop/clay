#pragma scop
/* Clay
   reorder([1], [0,2,1]);
*/
a = 0;
for(i = 0 ; i <= N ; i++) {
  b[i] = 0;
  for(j = 0 ; j <= M ; j++) {
    c[i][j] = 0;
  }
  for(k = 0 ; k <= P ; k++) {
    d[i][k] = 0;
  }
}
#pragma endscop
