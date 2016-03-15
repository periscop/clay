#pragma scop
/* Clay
   iss([0, 1], {1, 0 || 0, 0 | -10});
*/
for (i = 0 ; i <= N ; i++) {
  a[i] = 0;
  for (j = 0 ; j <= M ; j++) {
    b[i][j] = 0;
    c[i][j] = 0;
  }
  d[i] = 0;
}
e = 0;
#pragma endscop
