#pragma scop
/* Clay

a = get_array_id ("a");
b = get_array_id("b");

c = add_array("a_copy");
d = add_array("b_copy");

replace_array(a, c);
replace_array(b, d);

y = [0,0,0]; # beta for the statement a
z = [0,0,1]; # beta for the statement b

# copy data
datacopy(c, a, [0], true, y); # a_copy = a
y = [1,0,0];
z = [1,0,1];
datacopy(d, b, [0], true, z); # b_copy = b
fuse([0]);
fuse([0,0]);

# backup data
datacopy(a, c, [1], false, y); # a = a_copy
datacopy(b, d, [1], false, z); # b = b_copy
fuse([2]);
fuse([2,0]);

dimreorder([], c, [1, 0]);
dimreorder([], d, [1, 0]);

*/

for (i = 0 ; i <= N ; i++)
  for (j = 0 ; j <= M ; j++) {
    a[j][i]++;
    b[j][i] = 0;
  }

#pragma endscop


