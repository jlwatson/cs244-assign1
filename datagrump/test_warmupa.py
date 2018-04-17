#
# Tester for Warmup A
#
# Modifies fixed window size in controller.cc and extracts bandwidth and
# delay data from output
#

import subprocess
import pickle

RANGE_MIN, RANGE_MAX, RANGE_STEP = 5, 110, 5 

with open('controller.cc', 'r') as f:
  controller_lines = f.readlines()

results = []
for window_size in range(RANGE_MIN, RANGE_MAX, RANGE_STEP):
  new_line = "  unsigned int the_window_size = " + str(window_size) + ";\n"
  controller_lines[16] = new_line
  
  with open('controller.cc', 'w+') as new_f:
    new_f.writelines(controller_lines) 

  subprocess.run(['make'])
  complete = subprocess.run(
    ['./run-contest', 'test_warmup'],
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    universal_newlines = True
  )

  output = complete.stderr.split('\n')
  for l in output:
    if "Average throughput" in l:
      i1 = l.index(':')
      i2 = l.index(' Mbits/s')
      throughput = float(l[i1+2:i2-1])
    if "95th percentile signal delay" in l:
      i1 = l.index(':')
      i2 = l.index('ms')
      delay = int(l[i1+2:i2-1]) 

  results.append((window_size, throughput, delay))

print(results)
with open('warmupa_data.pickle', 'wb') as f:
  pickle.dump(results, f)
