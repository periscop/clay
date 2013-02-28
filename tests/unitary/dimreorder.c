#pragma scop
/* Clay
  dimreorder([0,0], 5, [1, 0]); # 5 == a
*/

for(i = 0 ; i <= N ; i++) {
  for (j = 0 ; j <= M ; j++) {
    a[i] [j*4] = a[i][j + N + 2*M + 5] * 2;
  }
}

#pragma endscop

