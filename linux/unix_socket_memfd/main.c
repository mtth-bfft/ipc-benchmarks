#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	    struct msghdr msgh = { 0 };
	    struct cmsghdr *cmsg;
	    union { // Ancillary data buffer, wrapped in a union in order to ensure it is suitably aligned
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	    } u;
	    char dummy[1] = { 0 };
	    struct iovec iov[1] = { 0 };
	    iov[0].iov_base = dummy;
	    iov[0].iov_len = sizeof(dummy);
	    msgh.msg_iov = iov;
	    msgh.msg_iovlen = sizeof(iov) / sizeof(iov[0]);
	    msgh.msg_control = u.buf;
	    msgh.msg_controllen = sizeof(u.buf);
	    cmsg = CMSG_FIRSTHDR(&msgh);
	    cmsg->cmsg_level = SOL_SOCKET;
	    cmsg->cmsg_type = SCM_RIGHTS;
	    cmsg->cmsg_len = CMSG_LEN(sizeof(int));

	    ssize_t bytes = recvmsg(socks[0], &msgh, 0);
	    if (bytes == 0) {
                close(socks[0]);
	        fprintf(stderr, " [.] Child exiting\n");
		return 0;
	    }
	    else if (bytes < 0) {
                fprintf(stderr, " [!] Read failed: code %d\n", errno);
	        return errno;
	    }
	    else if (cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS)
	    {
                fprintf(stderr, " [!] Invalid anciliary control message: %d | %d\n", cmsg->cmsg_level, cmsg->cmsg_type);
	        return errno;
	    }

	    int memfd = -1;
	    memcpy(&memfd, CMSG_DATA(cmsg), sizeof(int));
	    if (memfd < 0)
	    {
                fprintf(stderr, " [!] No file descriptor received, aborting\n");
	        return errno;
	    }

	    int seals = fcntl(memfd, F_GET_SEALS);
	    if (seals < 0)
	    {
                fprintf(stderr, " [!] Unable to fcntl(F_GET_SEALS): code %d\n", errno);
	        return errno;
	    }
	    else if ((seals & F_SEAL_SHRINK) == 0 || (seals & F_SEAL_GROW) == 0 || (seals & F_SEAL_WRITE) == 0)
	    {
                fprintf(stderr, " [!] Missing seal on memfd (0x%08X), aborting\n", seals);
	        return EPERM;
	    }
	    
	    void *memfd_map = mmap(NULL, buf_size, PROT_READ, MAP_PRIVATE, memfd, 0);
	    if (memfd_map == MAP_FAILED)
	    {
                fprintf(stderr, " [!] Unable to mmap memfd in child: code %d\n", errno);
	        return errno;
	    }

	    if (munmap(memfd_map, buf_size) < 0)
	    {
                fprintf(stderr, " [!] Unable to munmap memfd in child: code %d\n", errno);
	        return errno;
	    }
	    if (close(memfd) < 0)
	    {
                fprintf(stderr, " [!] Unable to close memfd in child: code %d\n", errno);
	        return errno;
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

	    int memfd = memfd_create("ipc_memfd", MFD_ALLOW_SEALING);
	    if (memfd < 0)
	    {
		fprintf(stderr, " [!] Unable to create memfd: code %d\n", errno);
		return errno;
	    }
	    if (ftruncate(memfd, buf_size) < 0)
	    {
		fprintf(stderr, " [!] Unable to set memfd size: code %d\n", errno);
		return errno;
	    }
	    void *memfd_map = mmap(NULL, buf_size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, 0);
	    if (memfd_map == MAP_FAILED)
	    {
		fprintf(stderr, " [!] Unable to mmap memfd: code %d\n", errno);
		return errno;
	    }
	    // Copying data here into the memfd is a bit awkward for this benchmark
	    // as other benchmarks only measure the time it takes to *send* the buffer,
	    // assuming the data is already prepared. In the case of a zero-copy IPC,
	    // the memfd filling part should not be included in the benchmark.
	    // => Uncomment this line to test "as if" in zero-copy mode
	    //memcpy(memfd_map, buf, buf_size);

	    if (munmap(memfd_map, buf_size) < 0)
	    {
		fprintf(stderr, " [!] Unable to munmap memfd: code %d\n", errno);
		return errno;
	    }
	    if (fcntl(memfd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE) < 0)
	    {
                fprintf(stderr, " [!] Unable to fcntl(F_ADD_SEALS): code %d\n", errno);
		return errno;
	    }

	    struct msghdr msgh = { 0 };
	    struct cmsghdr *cmsg;
	    union { // Ancillary data buffer, wrapped in a union in order to ensure it is suitably aligned
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	    } u;
	    char dummy[1] = { 0 };
	    struct iovec iov[1] = { 0 };
	    iov[0].iov_base = dummy;
	    iov[0].iov_len = sizeof(dummy);
	    msgh.msg_iov = iov;
	    msgh.msg_iovlen = sizeof(iov) / sizeof(iov[0]);
	    msgh.msg_control = u.buf;
	    msgh.msg_controllen = sizeof(u.buf);
	    cmsg = CMSG_FIRSTHDR(&msgh);
	    cmsg->cmsg_level = SOL_SOCKET;
	    cmsg->cmsg_type = SCM_RIGHTS;
	    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	    memcpy(CMSG_DATA(cmsg), &memfd, sizeof(int));

	    if (sendmsg(socks[1], &msgh, 0) < 0)
	    {
		fprintf(stderr, " [!] Sendmsg failed with code %d\n", errno);
		return errno;
	    }
	    close(memfd);

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

