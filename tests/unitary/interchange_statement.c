#pragma scop
/* Clay
   interchange([0,0,0,1], 1, 3, 0);
*/
for(i = 0 ; i <= N ; i++) {
  for(j = 0 ; j <= M ; j++) {
    for(k = 0 ; k <= P ; k++) {
      a[i][j][k] = 0;
      b[i][j][k] = 0;
      c[i][j][k] = 0;
    }
  }
}
#pragma endscop
