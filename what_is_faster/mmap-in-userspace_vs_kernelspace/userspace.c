#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	struct timespec start, end;
	long int result_in_ns;
	long int max_count = 250;
	int repeat_count = 20;

	int fd = open("/dev/fast_test", O_RDWR);
	if (fd < 0) {
		perror("open\n");
		return -1;
	}

	void *test_page = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!test_page) {
		perror("mmap\n");
		return -1;
	}

	for (int j = 0; j < repeat_count; j++) {
		memset(&start, 0, sizeof(struct timespec));
		memset(&end, 0, sizeof(struct timespec));

		clock_gettime(CLOCK_REALTIME, &start);
		for (int i = 0; i < max_count; i++) {
			*(int *)(test_page) = i;
			asm volatile("mfence":::"memory");
		}
		clock_gettime(CLOCK_REALTIME, &end);

		result_in_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

//		printf("%s():%d. WITH asm volatile thing: result = %ld [ns]\n", __FUNCTION__, __LINE__, result_in_ns);
		printf("%s():%d. WITH asm volatile thing: result divided by %ld = %ld [ns]\n", __FUNCTION__, __LINE__, max_count, result_in_ns / max_count);

/*		memset(&start, 0, sizeof(struct timespec));
		memset(&end, 0, sizeof(struct timespec));

		clock_gettime(CLOCK_REALTIME, &start);
		for (int i = 0; i < max_count; i++) {
			*(int *)(test_page) = i;
		}
		clock_gettime(CLOCK_REALTIME, &end);

		result_in_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

		printf("%s():%d. CLEAR: result = %ld [ns]\n", __FUNCTION__, __LINE__, result_in_ns);
		printf("%s():%d. CLEAR: result divided by %ld = %ld [ns]\n", __FUNCTION__, __LINE__, max_count, result_in_ns / max_count);*/

		usleep(2000);

	}

	munmap(test_page, 4096);
}
