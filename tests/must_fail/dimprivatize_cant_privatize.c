#pragma scop
/* Clay
   dimprivatize([0,0], 5, 2); # 5 == a
*/

for (i = 0 ; i <= N ; i++) {
  for (j = 0 ; j <= M ; j++) {
    a[i][j] = 0;
  }
}

#pragma endscop

