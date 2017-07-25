#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>

#define BUF_SIZE 1024

static int serial_open(char *device)
{
	int fd = -1;

	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0) {
		printf("Can't open device %s: %s\n", device, strerror(errno));
		return -1;
	}

    /*清除串口非阻塞标志*/
	if(fcntl(fd, F_SETFL, 0) < 0)
	{
		fprintf(stderr,"fcntl failed!\n");
		return -1;
	}

	return fd;
}

/*串口设置*/  
static int serial_set(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	/*保存原先配置，以测试配置是否正确、该串口是否可用等。调试成功，函数返回0，失败，函数返回-1*/
	if  (tcgetattr(fd,&oldtio) != 0) {
		perror("SetupSerial 1");
		return -1;
	}

	memset(&newtio,0,sizeof(newtio));
	/*设置控制模式*/
	newtio.c_cflag  |=  CLOCAL | CREAD;
	/*设置字符大小，没有现成可用函数，需要位掩码。一般先去除数据位中的位掩码，再重新按要求设置。*/
	newtio.c_cflag &= ~CSIZE;  

	/*设置数据位*/
    switch(nBits)
    {
	case 7:  /*在发送和接收字符时使用7个二进制位*/  
		newtio.c_cflag |= CS7;  
		break;
	case 8: /*在发送和接收字符时使用8个二进制位*/  
		newtio.c_cflag |= CS8;  
		break;
    }

	/*设置奇偶校验位*/ 
	switch(nEvent)
	{
	case 'o':
	case 'O':/*使能奇校验*/
		newtio.c_cflag |= PARENB;//激活校验码的生成和检测功能
		newtio.c_cflag |= PARODD;//使用奇校验而不是偶校验
		newtio.c_iflag |= (INPCK | ISTRIP);//INPCK对接收到的字符进行奇偶校验，ISTRIP把所有接收到的字符都设为7位二进制位 ?????
		break;
	case 'e':
	case 'E':
		newtio.c_iflag |= (INPCK | ISTRIP);
		newtio.c_cflag |= PARENB;
		newtio.c_cflag &= ~PARODD;//使用偶校验
		break;
	case 'n':
	case 'N':
		newtio.c_cflag &= ~PARENB;//无校验位
		break;
	default:
		break;
	}

	/*设置波特率*/
	switch(nSpeed)
	{
	case 2400:/*成功时返回0，失败-1*,进出波特率一样*/
		cfsetispeed(&newtio, B2400);
		cfsetospeed(&newtio, B2400);
		break;
	case 4800:
		cfsetispeed(&newtio, B4800);
		cfsetospeed(&newtio, B4800);
		break;
	case 9600:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	case 57600:
		cfsetispeed(&newtio, B57600);
		cfsetospeed(&newtio, B57600);
		break;
	case 115200:
		cfsetispeed(&newtio, B115200);
		cfsetospeed(&newtio, B115200);
		break;
	case 460800:
		cfsetispeed(&newtio, B460800);
		cfsetospeed(&newtio, B460800);
		break;
	default:
		cfsetispeed(&newtio, B9600);
		cfsetospeed(&newtio, B9600);
		break;
	}

	/*设置停止位*/
	switch(nStop) {
	case 1:
        newtio.c_cflag &=  ~CSTOPB;//使用一个停止位
		break;	
	case 2:
        newtio.c_cflag |= CSTOPB;
		break;
	default:
		printf("Unsupported stop bits\n");
		break;
	}

	/*设置输出模式为原始输出*/
	newtio.c_oflag &= ~OPOST;

	/*设置本地模式为原始模式*/
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

#if 0
	/*设置等待时间和最小接收字符*/
	newtio.c_cc[VTIME] =15;
	newtio.c_cc[VMIN] =2;
#endif

	/*处理要写入的引用对象，刷新收到的数据不读取*/
	tcflush(fd,TCIFLUSH);

	/*激活配置，激活的配置立即生效*/
	if((tcsetattr(fd,TCSANOW,&newtio)) != 0)
	{
		printf("com set error\n");
		return -1;
	}
    return 0;
}

static int serial_write(int fd)
{
	int wbyte = 0;

	wbyte = write(fd, "test", 4);

	return 0;
}

static int serial_read(int fd, char *buf, int count)
{
	int nleft;
	int nread;

	nleft = count;

	while(nleft > 0) {
		memset(buf, 0, sizeof(buf));
		if ((nread = read(fd, buf, nleft)) < 0) {
			/*被信号中断*/
			if (errno == EINTR)
				nread = 0;
			else 
				return -1;
		}else if(nread == 0) {
			break;
		}

		nleft -= nread;
		buf += nread;
	}

	return (count-nleft);
}

static int handle_read(int fd, char *buf)
{
	int rbyte = -1;
	int ret;
	fd_set rfds;
	struct timeval time;

	/*将文件描述符加入读描述符集合*/
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	time.tv_sec = 0;
	time.tv_usec = 100;

	ret = select(fd + 1, &rfds, NULL, NULL, &time);
	switch(ret) {
		case -1:
			fprintf(stderr, "select error!\n");
			break;
		case 0:
			//fprintf(stderr, "time out!\n");
			break;
		default:
			if (FD_ISSET(fd, &rfds)) {
				rbyte = serial_read(fd, buf, BUF_SIZE);	
				if(rbyte == -1) {
					fprintf(stderr, "serial_read error!\n");
				}
			}
			break;
	}

	return rbyte;
}

int main(int argc, char *argv[])
{
	int serial_fd = -1;
	int result;
	char buf[BUF_SIZE] = {0};
	char tmp[BUF_SIZE] = {0};
	int offset = 0;


	serial_fd = serial_open("/dev/ttyH0");
	if(serial_fd < 0) {
		printf("serial_open error!\n");
		exit(1);
	}

	serial_set(serial_fd, 115200, 8, 'O', 1);

	memset(buf, 0, sizeof(buf));
	//serial_write(serial_fd);
	while (1) {
		memset(tmp, 0, sizeof(tmp));
		result = handle_read(serial_fd, tmp);
		if (strlen(tmp) > 0)
			printf("tmp=%s\n", tmp);
		if(tmp[result -1] == '#') {
			memcpy(buf+offset, tmp, result);
			printf("buf=%s\n", buf);
			memset(buf, 0, sizeof(buf));
			offset = 0;
		}else if (result > 0){
			memcpy(buf+offset, tmp, result);
			offset += result;
		}

	//	if (result < 0) {
	//		printf("handle_read error!\n");
	//	}
	}

	close(serial_fd);
	return 0;
}
