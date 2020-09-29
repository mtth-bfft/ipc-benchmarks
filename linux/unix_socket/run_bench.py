#!/usr/bin/env python3

import math
import numpy as np
import os
import sys

def bench(msg_size, iterations=1000000):
    sys.stderr.write(' [.] Iteration: msg_size=%d\n' % msg_size)
    filename = '/tmp/bench_%d_bytes_%d_iter' % (msg_size, iterations)
    os.system('/bin/bash -c "./main %d %d > %s"' % (msg_size, iterations, filename))
    vec = np.genfromtxt(filename)
    sys.stdout.write('%d\t%d\t%d\t%d\n' % (msg_size, np.percentile(vec, 10), np.percentile(vec, 50), np.percentile(vec, 90)))
    sys.stdout.flush()

if __name__ == '__main__':
    log_scale = True
    if log_scale:
        subsampling = 20
        for i in range(0, 8):
            prev = -1
            for k in range(subsampling):
                msg_size = math.floor(math.exp(math.log(10)*(i + float(k)/subsampling)))
                if msg_size == prev:
                    continue
                prev = msg_size
                bench(msg_size)
    else:
        msg_size = 1
        while msg_size <= 0x10:
            bench(msg_size)
            msg_size *= 2
        while msg_size <= 10*1024:
            bench(msg_size)
            msg_size += 0x10
        while msg_size <= 10*1024*1024:
            bench(msg_size)
            msg_size += 1024

