import pickle
import matplotlib
import matplotlib.pyplot as plt

with open('warmupa_data.pickle', 'rb') as f:
  data = pickle.load(f)

windows, bandwidths, delays = [x[0] for x in data], \
                              [x[1] for x in data], \
                              [x[2] for x in data]

plt.plot(bandwidths, delays, 'bo', markersize=5)
for d in data:
  plt.annotate(d[0], xy = (d[1], d[2]))
plt.xlabel('Throughput (Mbits/s)')
plt.ylabel('95th-percentile signal latency (ms)')
plt.title('Warmup A - Throughput vs. Delay')

plt.show()

