#pragma scop
/* Clay
   reorder([], [0,1,2]);
*/
a = 0;
for(i = 0 ; i <= N ; i++) {
  b[i] = 0;
  c[i] = 0;
  for(j = 0 ; j <= M ; j++) {
    e[i][j] = 0;
  }
}
f = 0;
#pragma endscop
