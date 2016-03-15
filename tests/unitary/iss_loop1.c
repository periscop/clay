#pragma scop
/* Clay
   iss([0], {1 | 0 | 0 | -1});
*/
for (i = 0 ; i <= N ; i++) {
    a[i] = 0;
    b[i] = 0;
    c[i] = 0;
}
d = 0;
#pragma endscop
