#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PATH "/home/jihj/mygit/source_code/open_IO/test.log"
#define MAX_SIZE 60

typedef struct _logfile {
	int fd;
	int startPosition;
	int  stopPosition;
	off_t size;
}LOGFILE;

#if 1
/*这种方式是判断写入的数据超过最大值就重新从文件开始位置写入*/
static void write_log(LOGFILE *sysfile, int num)
{
	int wbyte = 0;
	int ret = 0;
	char buff[1024] = {0};

	memset(buff, 0, sizeof(buff));

	ret = sprintf(buff, "%s%d\r\n", "ttttttt", num);	

	/*写入文件内容之前判断上次写入数据结束地址+写入的数据>最大值?*/
	if ((sysfile->stopPosition + ret) > MAX_SIZE) {
		sysfile->startPosition = 0;
		wbyte = pwrite(sysfile->fd, buff, ret, sysfile->startPosition);
		sysfile->startPosition = sysfile->stopPosition = wbyte;
	}else {
		wbyte = pwrite(sysfile->fd, buff, ret, sysfile->startPosition);
		if (wbyte == -1) 
			printf("write error!\n");

		sysfile->startPosition = sysfile->stopPosition = sysfile->startPosition + wbyte;

		sysfile->size = lseek(sysfile->fd, 0, SEEK_END);

		if (sysfile->size == -1) 
			printf("lseek error:%s\n", strerror(errno));	
	}
}
#else
/*这种方式是写入的数据连续写入文件中*/
static void write_log(LOGFILE *sysfile, int num)
{
	int wbyte = 0;
	int ret = 0;
	int diff = 0, count = 0;
	char buff[1024] = {0};

	memset(buff, 0, sizeof(buff));

	ret = sprintf(buff, "%s%d\r\n", "ttttttt", num);	

	/*写入文件内容之前判断上次写入数据结束地址+写入的数据>最大值?*/
	if ((sysfile->stopPosition + ret) > MAX_SIZE) {
		diff = MAX_SIZE - sysfile->stopPosition;
		wbyte = pwrite(sysfile->fd, buff, diff, sysfile->startPosition);
		
		sysfile->startPosition = sysfile->stopPosition = 0;
		count = ret - diff;
		wbyte = pwrite(sysfile->fd, (buff+diff), count, sysfile->startPosition);
		sysfile->startPosition = sysfile->stopPosition = count;

		/*当超过文件最大值后,就不能用文件大小作为判断标志，应该用写入数据的最后位置作为判断标志*/
		//sysfile->size = lseek(sysfile->fd, 0, SEEK_END);
	}else {
		wbyte = pwrite(sysfile->fd, buff, ret, sysfile->startPosition);
		if (wbyte == -1) 
			printf("write error!\n");

		sysfile->startPosition = sysfile->stopPosition = sysfile->startPosition + wbyte;
		printf("sysfile->stopPosition=%d\n", sysfile->stopPosition);

		sysfile->size = lseek(sysfile->fd, 0, SEEK_END);
		printf("sysfile->size=%ld\n", sysfile->size);

		if (sysfile->size == -1) 
			printf("lseek error:%s\n", strerror(errno));	
	}
}
#endif

int main()
{
	int i;

	/*结构体无序初始化*/
	LOGFILE logfile = {
		.fd = -1,
		.startPosition = 0,
		.stopPosition = 0,
		.size = 0,
	};

	logfile.fd = open(PATH, O_RDWR | O_CREAT, 0777);
	if (logfile.fd < 0) {
		printf("open error:%s\n",strerror(errno));
		exit(1);
	}
	
	for (i = 10; i < 30; i++)
		write_log(&logfile, i);

	close (logfile.fd);
	return 0;
}
