#pragma scop
/* Clay
   dimcontract([0], 3, 1); # 3 == a
*/

for (i = 0 ; i <= N ; i++) {
  a[i] = 0;
  b[i] = a[i] + c[i];
}

#pragma endscop

