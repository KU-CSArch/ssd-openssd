// ***********************************************
// gunjae: copied from nvme_rw for test
// ***********************************************

#include <linux/nvme.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#define EN_CHECK_ERR 1
#define EN_WR (1 | EN_CHECK_ERR)
#define LBA_BASE 0x200
#define PAGE_PER_CMD 4
#define ITER 1

int main(int argc, char **argv)
{
	// for time measurement
	struct timespec time_start, time_end;
	double time_elapsed;

	static const char *perrstr;
	int err, fd;
	void *data_wr;
	void *data_rd;
	if ( posix_memalign(&data_wr, 4096, PAGE_PER_CMD*4096) ) {
		fprintf(stderr, "cannot allocate io payload for data_wr\n");
		return 0;
	}
	if ( posix_memalign(&data_rd, 4096, PAGE_PER_CMD*4096) ) {
		fprintf(stderr, "cannot allocate io payload for data_rd\n");
		return 0;
	}
	//char data[4096*PAGE_PER_CMD];
	//char data1[4096*PAGE_PER_CMD];

	unsigned int i = 0;
	unsigned int j = 0;
	for (i = 0; i < 4096*PAGE_PER_CMD; i++) {
		((char *)data_wr)[i] = (char)(i%256);
		//((char *)data_wr)[i] = (char)0;
		//data1[i] = (char)(i%128);
	}
	//for (i = 0; i < 256; i++) printf("%u ", ((char *)data_wr)[i]);
	printf("\n");

	struct nvme_user_io io;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <device>\n", argv[0]);
		return 1;
	}

	perrstr = argv[1];
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
		goto perror;

	io.opcode = nvme_cmd_write;
	io.flags = 2;//2;	// gunjae: flag test
	io.control = 0;
	io.metadata = (unsigned long)0;
	io.addr = (unsigned long)data_wr;
	io.slba = LBA_BASE;	// gunjae: should be modified??
	io.nblocks = (PAGE_PER_CMD-1);
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;

	unsigned long n_byte = 0;
	unsigned long n_err = 0;

#if (EN_WR)
	// gunjae: write commands
	clock_gettime(CLOCK_MONOTONIC, &time_start);
	for (i = 0; i < ITER; i++) {
		io.slba = LBA_BASE + (i*PAGE_PER_CMD);
		io.addr = (unsigned long)data_wr;
		//io.addr = (i%2) ? (unsigned long)data1: (unsigned long)data;
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		if (err < 0)
			goto perror;
		if (err)
			fprintf(stderr, "nvme write status:%x\n", err);
	}
	clock_gettime(CLOCK_MONOTONIC, &time_end);
	time_elapsed = ((double)time_end.tv_sec - (double)time_start.tv_sec);
	time_elapsed += ((double)time_end.tv_nsec - (double)time_start.tv_nsec) / 1000000000.0;
	fprintf(stdout, "\n");
	fprintf(stdout, "TIME_WR: %lf, bw=%lf B/s\n", time_elapsed, (double)4096*PAGE_PER_CMD*ITER/time_elapsed);
#endif	// EN_WR
	
	//sleep(10);
	
	// gunjae: read commands
	io.opcode = nvme_cmd_read;
	clock_gettime(CLOCK_MONOTONIC, &time_start);
	for (i = 0; i < ITER; i++) {
		io.slba = LBA_BASE + (i*PAGE_PER_CMD);
		io.addr = (unsigned long)data_rd;
		//io.addr = (i%2) ? (unsigned long)data1: (unsigned long)data;
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		if (err < 0)
			goto perror;
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	#if (EN_CHECK_ERR)
		for (j = 0; j < 4096*PAGE_PER_CMD; j++) {
			n_byte++;
			n_err += ( ((char *)data_rd)[j]!=(char)(j%256) ) ? 1: 0;
			//if (i%2)
			//	n_err += (data1[j]!=(char)(j%128)) ? 1: 0;
			//else
			//	n_err += (data[j]!=(char)(j%256)) ? 1: 0;
		}
	#endif	// EN_CHECK_ERR
	}
	clock_gettime(CLOCK_MONOTONIC, &time_end);
	time_elapsed = ((double)time_end.tv_sec - (double)time_start.tv_sec);
	time_elapsed += ((double)time_end.tv_nsec - (double)time_start.tv_nsec) / 1000000000.0;
	fprintf(stdout, "\n");
	fprintf(stdout, "TIME_RD: %lf, bw=%lf B/s\n", time_elapsed, (double)4096*PAGE_PER_CMD*ITER/time_elapsed);
#if (EN_CHECK_ERR)
	fprintf(stdout, "ERR: %ld / %ld = %lf \n", n_err, n_byte, (double)n_err/n_byte);
#endif

	//printf("\n");
	//for (i = 0; i < 4096; i++) printf("%u ", ((char *)data_rd)[i]);
	//printf("\n");
	return 0;

 perror:
	perror(perrstr);
	return 1;
}
