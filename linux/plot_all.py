#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np

data_np = np.genfromtxt('./unix_pipe/out_unix_pipe_1048576')
x_np = data_np[:,0]
y_latency_min_np = data_np[:,1]
y_latency_med_np = data_np[:,2]
y_latency_max_np = data_np[:,3]
y_bandwidth_min_np = (x_np*1000)/data_np[:,1]
y_bandwidth_med_np = (x_np*1000)/data_np[:,2]
y_bandwidth_max_np = (x_np*1000)/data_np[:,3]

data_socket = np.genfromtxt('./unix_socket/out_unix_socket')
x_socket = data_socket[:,0]
y_latency_min_socket = data_socket[:,1]
y_latency_med_socket = data_socket[:,2]
y_latency_max_socket = data_socket[:,3]
y_bandwidth_min_socket = (x_socket*1000)/data_socket[:,1]
y_bandwidth_med_socket = (x_socket*1000)/data_socket[:,2]
y_bandwidth_max_socket = (x_socket*1000)/data_socket[:,3]

data_memfd = np.genfromtxt('./unix_socket_memfd/out_unix_socket_memfd_without_zerocopy')
x_memfd = data_memfd[:,0]
y_latency_min_memfd = data_memfd[:,1]
y_latency_med_memfd = data_memfd[:,2]
y_latency_max_memfd = data_memfd[:,3]
y_bandwidth_min_memfd = (x_memfd*1000)/data_memfd[:,1]
y_bandwidth_med_memfd = (x_memfd*1000)/data_memfd[:,2]
y_bandwidth_max_memfd = (x_memfd*1000)/data_memfd[:,3]

data_memfd0copy = np.genfromtxt('./unix_socket_memfd/out_unix_socket_memfd_with_zerocopy')
x_memfd0copy = data_memfd0copy[:,0]
y_latency_min_memfd0copy = data_memfd0copy[:,1]
y_latency_med_memfd0copy = data_memfd0copy[:,2]
y_latency_max_memfd0copy = data_memfd0copy[:,3]
y_bandwidth_min_memfd0copy = (x_memfd0copy*1000)/data_memfd0copy[:,1]
y_bandwidth_med_memfd0copy = (x_memfd0copy*1000)/data_memfd0copy[:,2]
y_bandwidth_max_memfd0copy = (x_memfd0copy*1000)/data_memfd0copy[:,3]

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
ax1.set_title('Latency (ns) (10th and 90th percentiles)')
ax1.set_yscale('log')
ax2.set_xlabel('Message size (bytes)')
ax2.set_title('Bandwidth (MiB/s) (10th and 90th percentiles)')
ax2.set_yscale('log')
plt.xscale('log')

lines = []

ax1.plot(x_np, y_latency_min_np, color='C0', alpha=0.25)
lines += ax1.plot(x_np, y_latency_med_np, color='C0', marker='.')
ax1.plot(x_np, y_latency_max_np, color='C0', alpha=0.25)
ax1.fill_between(x_np, y_latency_min_np, y_latency_max_np, facecolor='blue', alpha=0.25, interpolate=True)

ax1.plot(x_socket, y_latency_min_socket, color='C1', alpha=0.25)
lines += ax1.plot(x_socket, y_latency_med_socket, color='C1', marker='.')
ax1.plot(x_socket, y_latency_max_socket, color='C1', alpha=0.25)
ax1.fill_between(x_socket, y_latency_min_socket, y_latency_max_socket, facecolor='orange', alpha=0.25, interpolate=True)

ax1.plot(x_memfd, y_latency_min_memfd, color='C2', alpha=0.25)
lines += ax1.plot(x_memfd, y_latency_med_memfd, color='C2', marker='.')
ax1.plot(x_memfd, y_latency_max_memfd, color='C2', alpha=0.25)
ax1.fill_between(x_memfd, y_latency_min_memfd, y_latency_max_memfd, facecolor='brown', alpha=0.25, interpolate=True)

ax1.plot(x_memfd0copy, y_latency_min_memfd0copy, color='C3', alpha=0.25)
lines += ax1.plot(x_memfd0copy, y_latency_med_memfd0copy, color='C3', marker='.')
ax1.plot(x_memfd0copy, y_latency_max_memfd0copy, color='C3', alpha=0.25)
ax1.fill_between(x_memfd0copy, y_latency_min_memfd0copy, y_latency_max_memfd0copy, facecolor='red', alpha=0.25, interpolate=True)

ax2.plot(x_np, y_bandwidth_min_np, color='C0', alpha=0.25)
ax2.plot(x_np, y_bandwidth_med_np, color='C0', marker='.')
ax2.plot(x_np, y_bandwidth_max_np, color='C0', alpha=0.25)
ax2.fill_between(x_np, y_bandwidth_min_np, y_bandwidth_max_np, facecolor='blue', alpha=0.25, interpolate=True)

ax2.plot(x_socket, y_bandwidth_min_socket, color='C1', alpha=0.25)
ax2.plot(x_socket, y_bandwidth_med_socket, color='C1', marker='.')
ax2.plot(x_socket, y_bandwidth_max_socket, color='C1', alpha=0.25)
ax2.fill_between(x_socket, y_bandwidth_min_socket, y_bandwidth_max_socket, facecolor='orange', alpha=0.25, interpolate=True)

ax2.plot(x_memfd, y_bandwidth_min_memfd, color='C2', alpha=0.25)
ax2.plot(x_memfd, y_bandwidth_med_memfd, color='C2', marker='.')
ax2.plot(x_memfd, y_bandwidth_max_memfd, color='C2', alpha=0.25)
ax2.fill_between(x_memfd, y_bandwidth_min_memfd, y_bandwidth_max_memfd, facecolor='brown', alpha=0.25, interpolate=True)

ax2.plot(x_memfd0copy, y_bandwidth_min_memfd0copy, color='C3', alpha=0.25)
ax2.plot(x_memfd0copy, y_bandwidth_med_memfd0copy, color='C3', marker='.')
ax2.plot(x_memfd0copy, y_bandwidth_max_memfd0copy, color='C3', alpha=0.25)
ax2.fill_between(x_memfd0copy, y_bandwidth_min_memfd0copy, y_bandwidth_max_memfd0copy, facecolor='red', alpha=0.25, interpolate=True)

ax1.legend(lines, ('Pipe', 'Socket', 'Sealed memfd without zerocopy', 'Sealed memfd with zerocopy'))
ax2.set_ylim(ax2.get_ylim()[0], 12000)
plt.show()

