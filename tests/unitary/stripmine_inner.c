#pragma scop
/* Clay
   stripmine([0,0], 2, 32);
*/
for(i = 0 ; i <= N ; i++) {
  for(j = 0 ; j <= M ; j++) {
    a[i][j] = 0;
    b[i][j] = 0;
    c[i][j] = 0;
  }
}
#pragma endscop
