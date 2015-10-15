#pragma scop
/* Clay
   skew([0,0,0], 3, 1, 2);
*/
for(k = 0 ; k < P ; k++) {
  for(i = 0 ; i <= N ; i++) {
    a[i][k] = 0;
    for(j = 0 ; j <= M ; j++) {
      b[i][j][k] = 0;
    }
  }
}
#pragma endscop
