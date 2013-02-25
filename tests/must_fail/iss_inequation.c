#pragma scop
/* Clay
   iss([0], {0 | 1 | 0 | 0});
*/
for (i = 0 ; i < N ; i++)
  a[i] = 0;
#pragma endscop
