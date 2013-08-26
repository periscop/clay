#!/usr/bin/python 


import sys
import os
import subprocess
import string


# Check arguments
if (len(sys.argv) < 3):
	print 'Usage:', sys.argv[0], 'source.c source.c.scop [clay_path] [clay options...]'
	sys.exit(2)

# Get arguments
source_filename = sys.argv[1]
scop_filename = sys.argv[2]
	
# Display arguments
print sys.argv[0], source_filename, scop_filename

# Check if source_filename and scop_filename exist
if (os.path.exists(source_filename) == False):
	print 'Error:', source_filename, ' file does not exist'
	sys.exit(3)
if (os.path.exists(scop_filename) == False):
	print 'Error:', scop_filename, ' file does not exist'
	sys.exit(4)

# Custom clay
clay = ""
if (len(sys.argv) >= 4):# and os.path.exists(sys.argv[3]) and os.access(sys.argv[3], os.X_OK)):
	clay = sys.argv[3]
else:
	clay = "clay"
# clay options
clay_options = ["--readc"]
if (len(sys.argv) > 4):
	i = 4
	while (i < len(sys.argv)):
		clay_options += [sys.argv[i]]
		i += 1
# Final clay
print "clay =", clay, clay_options


# Get scop form clay
clay_output,clay_error_output = subprocess.Popen([clay, source_filename] + clay_options, stdout = subprocess.PIPE, stderr = subprocess.PIPE).communicate()
clay_output += clay_error_output
#print clay_output

# Get scop
scop_file = open(scop_filename, 'r')
scop = ""
for line in scop_file:
	#sys.stdout.write(line)
	scop += line
scop_file.close()


# Compare clay_output and scop
s0 = ""
s1 = ""
# Remove empty line
skip_next_line = False
for line in string.split(clay_output, '\n'):
	if (skip_next_line):
		skip_next_line = False
		continue
	if (line == '# File name'): skip_next_line = True
	if line != '' and (string.find(line, 'enerated by') == -1): s0 += line + '\n'
for line in string.split(scop, '\n'):
	if (skip_next_line):
		skip_next_line = False
		continue
	if (line == '# File name'): skip_next_line = True
	if line != '' and (string.find(line, 'enerated by') == -1): s1 += line + '\n'
# Print
print s0
print s1
# Result
if (s0 != s1):
	print 'Result:', '"clay', source_filename + '"', 'and', '"' + scop_filename + '"', 'are different'
	sys.exit(1)
else:
	print 'Result:', '"clay', source_filename + '"', 'and', '"' + scop_filename + '"', 'have no difference'

# End
sys.exit(0)
