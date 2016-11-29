#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>

#include "IOTest.h"

int
main (int argc, char *argv[])
{
	int i;
	char fname[512];
	char *str = "Refmon, are you getting this?";
	printf("Confining after init\n");

	Java_IOTest_lwCConfine (NULL, NULL);

/*	for (i = 0; i < 10000; i++)
	{
		sprintf(fname, "/home/elnikety/workspace/snap/jvmrefmon/tmpfiles/%d.iotest.txt", i);
		FILE *fp = fopen(fname, "ab+");
		if (!fp) {
			printf("fopen for %s\n", fname);
			perror("fopen");
		}
		assert (fp);
		int ret = fwrite(str, strlen(str), 1, fp);
		assert (ret == 1);
		fclose(fp);
	}
*/


	for (i = 0; i < 10000; i++)
	{
		sprintf(fname, "/home/elnikety/workspace/snap/jvmrefmon/tmpfiles/%d.iotest.txt", i);
		int fd = open(fname, O_CREAT | O_RDWR);
		if (fd <= 0) {
			printf("open for %s\n", fname);
			perror("open");
		}
		assert (fd >= 1);
		int ret = write (fd, str, strlen(str));
		assert (ret == strlen(str));
		close(fd);
	}


	printf("Test done!\n");

	return 0;
}
