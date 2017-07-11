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

int main(int argc, char **argv)
{
	static const char *perrstr;
	int err, fd;
	char data[4096];


	int i = 0;
	for (i = 0; i < 100; i++) {
		data[i] = (char)i;
	}
	for (i = 0; i < 100; i++) printf("%u ", data[i]);
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
	io.addr = (unsigned long)data;
	io.slba = 0x501;	// gunjae: should be modified??
	io.nblocks = 0;
	io.dsmgmt = 0;
	io.reftag = 0;
	io.apptag = 0;
	io.appmask = 0;

	err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
	if (err < 0)
		goto perror;
	if (err)
		fprintf(stderr, "nvme write status:%x\n", err);

	for (i = 0; i < 2000; i++) {
		io.opcode = nvme_cmd_read;
		err = ioctl(fd, NVME_IOCTL_SUBMIT_IO, &io);
		if (err < 0)
			goto perror;
		if (err)
			fprintf(stderr, "nvme read status:%x\n", err);
	}

	printf("\n");
	for (i = 0; i < 100; i++) printf("%u ", data[i]);
	return 0;

 perror:
	perror(perrstr);
	return 1;
}
