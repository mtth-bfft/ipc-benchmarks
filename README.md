# IPC Benchmarks

These benchmarks are designed to be reproducible on a N-core (N >= 2) processor with hyperthreading disabled.

They were ran on a Intel(R) Core(TM) i5-4310U CPU @ 2.00GHz with hyperthreading and Intel SpeedStep disabled, using a 5.7.0 Linux kernel with the following (default) sysctls :

```
	net.core.wmem_default = 212992
	net.core.wmem_max = 212992
	net.core.rmem_default = 212992
	net.core.rmem_max = 212992
	fs.pipe-max-size = 1048576
	fs.pipe-user-pages-hard = 0
	fs.pipe-user-pages-soft = 16384
```

## Linux

The results are:
- using a UNIX socket instead of a pipe results in slightly higher latency, thus slightly lower bandwidth;
- using a sealed memory file descriptor is more efficient than other mechanisms above 50 to 60 KiB, but only when used as a zero-copy mechanism;
- in fact, using a sealed memory file descriptor not as a zero-copy mechanism is the least efficient mechanism tested, in all cases (even for large messages).

## Windows

WIP

