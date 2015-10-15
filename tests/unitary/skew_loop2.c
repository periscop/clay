#pragma scop
/* Clay
   skew([0,0,0], 3, 1, 2);
*/
for(i = 0 ; i <= N ; i++) {
  for(j = 0 ; j <= M ; j++) {
    for(k = 0 ; k <= P ; k++) {
      a[i][j][k] = 0;
    }
  }
}
#pragma endscop
