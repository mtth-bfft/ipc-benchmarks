#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

data = np.genfromtxt('./out_unix_socket_low_sizes')
x = data[:,0]
y_latency_min = data[:,1]
y_latency_med = data[:,2]
y_latency_max = data[:,3]
y_bandwidth_min = (x*1000)/data[:,1]
y_bandwidth_med = (x*1000)/data[:,2]
y_bandwidth_max = (x*1000)/data[:,3]

fig, ax1 = plt.subplots()

ax1.plot(x, y_latency_min, color='C0', alpha=0.25)
ax1.plot(x, y_latency_med, color='C0', marker='.')
ax1.plot(x, y_latency_max, color='C0', alpha=0.25)
ax1.fill_between(x, y_latency_min, y_latency_max, facecolor='blue', alpha=0.25, interpolate=True)
ax1.set_xticks(np.arange(0, 4096*3, 4096))
ax1.grid(which='major', alpha=0.5)
ax1.set_title('UNIX Socket Latency (ns) (10th and 90th percentiles)')
ax1.set_xlabel('Message size (bytes)')

plt.show()

