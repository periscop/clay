#pragma scop
/* Clay
block([0,0], [1,0]);
*/

for (i = 0 ; i <= N ; i++) 
  a[i] = 0;

for (i = 0 ; i <= N ; i++)
  b[i] = 0;

#pragma endscop


