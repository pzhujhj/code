#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define MEM_CLEAR 0x01
#define CHAR_DEV_NAME "/dev/chr_dev"

int main()
{
        int fd;
        char buf[32];

		/* open device */
        fd = open(CHAR_DEV_NAME, O_RDWR | O_NONBLOCK);
        if(fd < 0)
        {
                printf("open failed!\n");
                return -1;
        }

		strcpy(buf, "this is test");
		/* write data */
        write(fd, buf, 32);
        memset(buf, 0, 32);

		/* read data */
        lseek(fd, 0, SEEK_SET);
        read(fd, buf, 32);
		printf("recv buf=%s\n", buf);

		//ioctl(fd, MEM_CLEAR, NULL);
		lseek(fd, 0, SEEK_SET);  
        read(fd, buf, 32);

		printf("ioctl buf=%s\n", buf);

        close(fd);
        return 0;
}

