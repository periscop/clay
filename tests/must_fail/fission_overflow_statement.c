#pragma scop
/* Clay
   fission([0,1], 2);
*/
for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
  b[i] = 0;
  c[i] = 0;
}
#pragma endscop
