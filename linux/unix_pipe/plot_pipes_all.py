#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

data_65536 = np.genfromtxt('./out_unix_pipe_65536')
data_1048576 = np.genfromtxt('./out_unix_pipe_1048576')

x_65536 = data_65536[:,0]
y_latency_min_65536 = data_65536[:,1]
y_latency_med_65536 = data_65536[:,2]
y_latency_max_65536 = data_65536[:,3]
y_bandwidth_min_65536 = (x_65536*1000)/data_65536[:,1]
y_bandwidth_med_65536 = (x_65536*1000)/data_65536[:,2]
y_bandwidth_max_65536 = (x_65536*1000)/data_65536[:,3]

x_1048576 = data_1048576[:,0]
y_latency_min_1048576 = data_1048576[:,1]
y_latency_med_1048576 = data_1048576[:,2]
y_latency_max_1048576 = data_1048576[:,3]
y_bandwidth_min_1048576 = (x_1048576*1000)/data_1048576[:,1]
y_bandwidth_med_1048576 = (x_1048576*1000)/data_1048576[:,2]
y_bandwidth_max_1048576 = (x_1048576*1000)/data_1048576[:,3]

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

lines = []
ax1.plot(x_65536, y_latency_min_65536, color='C0', alpha=0.25)
lines += ax1.plot(x_65536, y_latency_med_65536, color='C0', marker='.')
ax1.plot(x_65536, y_latency_max_65536, color='C0', alpha=0.25)
ax1.fill_between(x_65536, y_latency_min_65536, y_latency_max_65536, facecolor='blue', alpha=0.25, interpolate=True)
ax1.plot(x_1048576, y_latency_min_1048576, color='C1', alpha=0.25)
lines += ax1.plot(x_1048576, y_latency_med_1048576, color='C1', marker='.')
ax1.plot(x_1048576, y_latency_max_1048576, color='C1', alpha=0.25)
ax1.fill_between(x_65536, y_latency_min_65536, y_latency_max_65536, facecolor='blue', alpha=0.25, interpolate=True)
ax1.fill_between(x_1048576, y_latency_min_1048576, y_latency_max_1048576, facecolor='orange', alpha=0.25, interpolate=True)
ax1.set_title('Latency (ns) (10th and 90th percentiles)')
ax1.set_yscale('log')

ax1.legend(lines, ('PIPE_SZ=65536 (default)', 'PIPE_SZ=1048576 (max)'))

ax2.plot(x_65536, y_bandwidth_min_65536, color='C0', alpha=0.25)
ax2.plot(x_65536, y_bandwidth_med_65536, color='C0', marker='.')
ax2.plot(x_65536, y_bandwidth_max_65536, color='C0', alpha=0.25)
ax2.fill_between(x_65536, y_bandwidth_min_65536, y_bandwidth_max_65536, facecolor='blue', alpha=0.25, interpolate=True)
ax2.plot(x_1048576, y_bandwidth_min_1048576, color='C1', alpha=0.25)
ax2.plot(x_1048576, y_bandwidth_med_1048576, color='C1', marker='.')
ax2.plot(x_1048576, y_bandwidth_max_1048576, color='C1', alpha=0.25)
ax2.fill_between(x_1048576, y_bandwidth_min_1048576, y_bandwidth_max_1048576, facecolor='orange', alpha=0.25, interpolate=True)
ax2.set_title('Bandwidth (MiB/s) (10th and 90th percentiles)')
ax2.set_xlabel('Message size (bytes)')
plt.xscale('log')

plt.show()

