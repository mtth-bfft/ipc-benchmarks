#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int main(int argc, const char* argv[])
{
    fprintf(stderr, " [.] Started\n");

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <buffer size> <message count>\n", argv[0]);
	return 1;
    }

    errno = 0;
    if (nice(-15) == -1 && errno != 0)
    {
        fprintf(stderr, " [!] Unable to set niceness: code %d\n", errno);
	return errno;
    }

    size_t buf_size = 0;
    long long res = atoll(argv[1]);
    if (res <= 0)
    {
        fprintf(stderr, "Invalid buffer size\n");
	return 1;
    }
    buf_size = (size_t)res;
    fprintf(stderr, " [.] Buffer size: %zu\n", buf_size);
    
    size_t msg_count = 0;
    res = atoll(argv[2]);
    if (res <= 0)
    {
        fprintf(stderr, "Invalid message count\n");
	return 1;
    }
    msg_count = (size_t)res;
    fprintf(stderr, " [.] Message count: %zu\n", msg_count);
    
    uint8_t *buf = malloc(buf_size);
    srand(0);
    for (size_t i = 0; i < buf_size; i++)
    {
        buf[i] = rand() % 256;
    }
    
    int socks[2] = { 0 };
    int err = socketpair(AF_UNIX, SOCK_STREAM, 0, socks);
    if (err != 0)
    {
        fprintf(stderr, " [!] Unable to create sockets: code %d\n", errno);
	return errno;
    }

    cpu_set_t *cpuset = CPU_ALLOC(2);
    CPU_ZERO_S(2, cpuset);
    CPU_SET_S(0, 2, cpuset);
    if (sched_setaffinity(0, CPU_ALLOC_SIZE(2), cpuset) != 0)
    {
        fprintf(stderr, " [!] Unable to pin parent to CPU: code %d\n", errno);
	return errno;
    }

    volatile struct timespec *timings = mmap(NULL, 2 * sizeof(struct timespec) * msg_count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (timings == NULL)
    {
        fprintf(stderr, " [!] Unable to allocate shared memory: code %d\n", errno);
	return errno;
    }

    int child_pid = fork();
    if (child_pid == -1)
    {
        fprintf(stderr, " [!] Fork failed: code %d\n", errno);
	return errno;
    }
    else if (child_pid == 0)
    {
        fprintf(stderr, " [.] Child running\n");
	close(socks[1]);

        CPU_ZERO_S(2, cpuset);
        CPU_SET_S(1, 2, cpuset);
        if (sched_setaffinity(0, CPU_ALLOC_SIZE(2), cpuset) != 0)
        {
            fprintf(stderr, " [!] Unable to pin parent to CPU: code %d\n", errno);
            return errno;
        }

	for (size_t i = 0; ; i++)
	{
	    size_t total_received = 0;

	    while (total_received != buf_size) {
	    	ssize_t bytes = read(socks[0], buf, buf_size - total_received);
	    	if (bytes == 0) {
                    close(socks[0]);
	            fprintf(stderr, " [.] Child exiting\n");
		    return 0;
	    	}
	        else if (bytes < 0) {
                    fprintf(stderr, " [!] Read failed: code %d\n", errno);
		    return errno;
	        }
		total_received += bytes;
	    }
	    timings[2 * i + 1].tv_sec = 1; // signal the parent we received this message
	}
    }
    else {
	close(socks[0]);
        for (size_t i = 0; i < msg_count; i++)
	{
	    if ((i % 1000) == 0)
	    {
		fprintf(stderr, "%.2f\r", (i * 100.0) / (msg_count));
		fflush(stderr);
	    }
    	    if (clock_gettime(CLOCK_MONOTONIC_RAW, (struct timespec*)&(timings[2 * i])) != 0)
    	    {
                fprintf(stderr, " [!] Unable to get clock: code %d\n", errno);
	        return errno;
    	    }
	    ssize_t bytes = write(socks[1], buf, buf_size);
	    if (bytes != buf_size)
	    {
                fprintf(stderr, " [!] Write failed: code %d\n", errno);
		break;
	    }
	    while (timings[2 * i + 1].tv_sec == 0 && timings[2 * i + 1].tv_nsec == 0) {
		// Busy loop a few times for the child to receive the message
	    }
	    // We have to timestamp this moment in the parent, since parent and child run on
	    // separate CPU cores which have desynchronised high-precision timers
    	    if (clock_gettime(CLOCK_MONOTONIC_RAW, (struct timespec*)&(timings[2 * i + 1])) != 0)
    	    {
                fprintf(stderr, " [!] Unable to get clock: code %d\n", errno);
	        return errno;
    	    }
	}
	close(socks[1]);
        wait(NULL);

        for (size_t i = 0; i < msg_count; i++)
        {
            uint64_t diff = ((uint64_t)(timings[2 * i + 1].tv_sec - timings[2 * i].tv_sec)) * 10000000000 + (timings[2 * i + 1].tv_nsec - timings[2 * i].tv_nsec);
            printf("%" PRIu64 "\n", diff);
        }
        fprintf(stderr, " [.] Parent exiting\n");
    }

    return 0;
}

