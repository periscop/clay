#pragma scop
/* Clay
   interchange([0,0,1], 3, 1, 0);
*/
for(i = 0 ; i <= N ; i++) {
  for(j = 0 ; j <= M ; j++) {
    a[i][j] = 0;
    for(k = 0 ; k <= P ; k++) {
      b[i][j][k] = 0;
    }
  }
}
#pragma endscop
