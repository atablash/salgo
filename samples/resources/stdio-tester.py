#!/usr/bin/python

############
# This program tests if given executable produces correct output for given input
#



import argparse
from os import listdir, path
from subprocess import Popen, PIPE
from sys import stdout
import time
from os import devnull
from decimal import Decimal


parser = argparse.ArgumentParser()
parser.add_argument('command', default='a.out')
parser.add_argument('tests', help='path to tests or directory with tests', default='.', nargs='*')
parser.add_argument('--generate', dest='generate', action='store_true', help="generate output if there's no OUT file")
parser.add_argument('--tolerance', default='0', help='relative or absolute error tolerance')
args = parser.parse_args()




args.tolerance = Decimal(args.tolerance)




if path.exists(args.command):
	command = './' + args.command
else:
	command = args.command






import re

def natural_sorted(l): 
    convert = lambda text: int(text) if text.isdigit() else text.lower() 
    alphanum_key = lambda key: [ convert(c) for c in re.split('([0-9]+)', key) ] 
    return sorted(l, key = alphanum_key)






tests = []

def append_tests(t):
	if path.isdir(t):
		for p in natural_sorted(listdir(t)):
			append_tests(path.join(t, p))
	elif path.exists(t) or args.generate:
		tests.append(t)
	else:
		print(t + ' does not exist')
		exit()


for test in args.tests:
	append_tests(test)



max_error = Decimal(0)

compare_lines_output = ''

def compare_lines(line_a, line_b):
	global compare_lines_output
	compare_lines_output = ''

	aa = line_a.split()
	bb = line_b.split()

	if len(aa) != len(bb):
		stdout.write('(number of tokens: ' + str(len(aa)) + ' vs ' + str(len(bb)) + ')->')
		return False

	i = 0
	for a,b in zip(aa,bb):
		i += 1
		if a != b:
			if args.tolerance != 0:
				given = Decimal(a)
				expected = Decimal(b)
				error = (expected - given) / max(expected, 1)

				global max_error
				max_error = max(max_error, error)

				if error < args.tolerance:
					if error != 0:
						compare_lines_output = 'diff: ' + str(error)
				else:
					compare_lines_output = 'ERROR: ' + str(error) + ' bigger than tolerance ' + str(args.tolerance)
					return False
			else:
				compare_lines_output = 'token #' + str(i) + ': ' + a + ' should be ' + b
				return False

	return True



num_tests = 0
time_sum = 0
time_max = 0


for test in tests:
	good = False

	if test[-3:] == ".in":
		good = True

	if path.basename(test)[0:2] == "in":
		good = True

	if '.in.' in path.basename(test):
		good = True

	if not good:
		continue


	stdout.write(test + '... ')
	stdout.flush()




	# get output file name
	out_file = test

	if out_file[-3:] == ".in":
		out_file = out_file[:-3] + ".out"
	
	if path.basename(out_file)[0:2] == "in":
		basename = 'out' + path.basename(out_file)[2:]
		out_file = path.join(path.dirname(out_file), basename)

	out_file = out_file.replace('/in/', '/out/')

	out_file = out_file.replace('.in.', '.out.')



	if not args.generate and not path.exists(out_file):
		print(out_file + ' does not exist')
		exit(1)

	if args.generate and path.exists(out_file):
		print(out_file + ' already exists')
		exit(1)








	t0 = time.time()

	where_to_stdout = PIPE
	if args.generate:
		where_to_stdout = open(out_file, 'w')

	process = Popen(command, stdout=where_to_stdout, stdin=open(test,'r'), stderr=open(devnull, 'w'), shell=True)

	

	err = False

	if not args.generate:
		with open(out_file, 'r') as f: 
			iline = 0
			for line in process.stdout:
				iline += 1
				
				if err:
					continue

				r = compare_lines(line, f.next())
				if not r:
					stdout.write('(diff on line ' + str(iline) + ') ')
					stdout.flush()
					err = True

				if compare_lines_output != '':
					stdout.write('(' + compare_lines_output + ') ')
					stdout.flush()

			try:
				while True:
					next_line = f.next()
					if not compare_lines('', next_line):
						stdout.write('(missing output, expected: ' + next_line + ')')
						stdout.flush()
						err = True
			except Exception:
				pass
		
	process.wait()

	time_diff = time.time() - t0

	print("%.3fs" % time_diff)

	num_tests += 1
	time_sum += time_diff
	time_max = max(time_max, time_diff)

	if err:
		break

print('')
print(' - total tests passed:  ' + str(num_tests))

if num_tests > 0:
	print(' - average test time:   %.3fs' % (time_sum / num_tests))
	print(' - max test time:       %.3fs' % time_max)


if args.tolerance != 0:
	print(' - max error: ' + str(max_error) + ' < tolerance ' + str(args.tolerance))
	

import sys

if err:
    sys.exit(1)

sys.exit(0)


