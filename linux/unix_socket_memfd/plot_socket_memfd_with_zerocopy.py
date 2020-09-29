#!/usr/bin/env python3

import matplotlib.pyplot as plt
from matplotlib.ticker import FormatStrFormatter, EngFormatter
import itertools
import numpy as np

data = np.genfromtxt('./out_unix_socket_memfd_with_zerocopy')

x = data[:,0]
y_latency_min = data[:,1]
y_latency_med = data[:,2]
y_latency_max = data[:,3]
y_bandwidth_min = (x*1000)/data[:,1]
y_bandwidth_med = (x*1000)/data[:,2]
y_bandwidth_max = (x*1000)/data[:,3]

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

ax1.yaxis.set_major_formatter(EngFormatter(unit='s'))
ax1.yaxis.set_minor_formatter(EngFormatter(unit='s'))

ax1.plot(x, y_latency_min, color='C0', alpha=0.25)
ax1.plot(x, y_latency_med, color='C0', marker='.')
ax1.plot(x, y_latency_max, color='C0', alpha=0.25)
ax1.fill_between(x, y_latency_min, y_latency_max, facecolor='blue', alpha=0.25, interpolate=True)
ax1.set_title('Sealed memfd Zero-Copy Latency (ns) (10th and 90th percentiles)')
ax1.set_yscale('log')

ax2.plot(x, y_bandwidth_min, color='C0', alpha=0.25)
ax2.plot(x, y_bandwidth_med, color='C0', marker='.')
ax2.plot(x, y_bandwidth_max, color='C0', alpha=0.25)
ax2.fill_between(x, y_bandwidth_min, y_bandwidth_max, facecolor='blue', alpha=0.25, interpolate=True)
ax2.set_title('Sealed memfd Zero-Copy Bandwidth (MiB/s) (10th and 90th percentiles)')
ax2.set_xlabel('Message size (bytes)')
ax2.set_yscale('log')
plt.xscale('log')
plt.axhspan(7e3, 999e3, facecolor='grey', alpha=0.25, zorder=-100)
ax2.annotate('(Theoretical bandwidth, input feed through CPU/memory controller/RAM becomes the bottleneck)',
        xy=(131000, 12000), xytext=(90, 150000), arrowprops={'facecolor': 'black', 'width': 0.5, 'headlength': 6, 'headwidth': 8},
        style='italic', fontsize=9)

plt.show()

