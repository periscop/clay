#pragma scop
/* Clay
   shift([0], 1, 1);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
}
#pragma endscop
