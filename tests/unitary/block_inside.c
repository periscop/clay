#pragma scop
/* Clay
   block([2,0], [4,0]);
 */
for (i = 0; i <= N; i++)
  a[i]++;

for (i = 0; i <= N; i++)
  b[i]++;

for (i = 0; i <= N; i++)
  c[i]++;

for (i = 0; i <= N; i++)
  d[i]++;

for (i = 0; i <= N; i++)
  e[i]++;

for (i = 0; i <= N; i++)
  f[i]++;
#pragma endscop
