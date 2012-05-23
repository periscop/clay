#pragma scop
/* Clay
   iss([0], [1,0,-10]);
*/
for(i = 0 ; i <= N ; i++) {
  for(j = 0 ; j <= M ; j++) {
    a[i][j] = 0;
  }
}
#pragma endscop
