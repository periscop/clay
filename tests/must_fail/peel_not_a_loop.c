#pragma scop
/* Clay
   peel([0,0], {10});
 */
for (i = 0; i < 100; i++) {
  a[i]++;
}
#pragma endscop
