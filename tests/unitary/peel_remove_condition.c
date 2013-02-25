#pragma scop
/* Clay
   peel([0], {-15});
*/

for(i = 0 ; i <= N ; i++) {
  a[i] = 0;
  if (i >= 15)
    b[i] = 0;
}

#pragma endscop
