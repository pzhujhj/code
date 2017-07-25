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

#define BUF_SIZE 512

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
#if 1
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
#else

	struct termios t;
	if (tcgetattr(fd, &t) < 0) {
		printf("Can't tcgetattr for %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	cfmakeraw(&t);
	/* 设置成基校验模式 */
	t.c_cflag |= (PARODD | PARENB);
	t.c_iflag |= INPCK;
	/* 设置波特率为115200 */
	cfsetispeed(&t, B115200);
	cfsetospeed(&t, B115200);

	if (tcsetattr(fd, TCSAFLUSH, &t) < 0) {
		printf("Can't tcsetattr for %s: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	tcflush(fd, TCIFLUSH);
#endif
    return 0;
}

ssize_t tread(int fd, char *buf, size_t nbytes, unsigned int stimeout, unsigned int utimeout)
{
	int nfds, rbyte = 0;
	fd_set readfds;
	struct timeval tv;

	tv.tv_sec = stimeout;
	tv.tv_usec = utimeout;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	nfds = select(fd+1, &readfds, NULL, NULL, &tv);

	switch(nfds) {
		case -1:
			fprintf(stderr, "select error!\n");
			break;
		case 0:
			//fprintf(stderr, "time out!\n");
			rbyte = -1;
			break;
		default:
			if (FD_ISSET(fd, &readfds)) {
				rbyte = read(fd, buf, nbytes);	
				if(rbyte == -1) {
					fprintf(stderr, "serial_read error!\n");
				}
			}
			break;
	}
	return rbyte;
}

ssize_t treadn(int fd, char *buf, size_t nbytes, unsigned int stimeout, unsigned int utimeout)
{
	size_t      nleft;
	ssize_t     nread;

	nleft = nbytes;

	while (nleft > 0) {
		if ((nread = tread(fd, buf, nleft, stimeout, utimeout)) < 0) {
			if (nleft == nbytes)
				return(-1);
			else
				break;
		}else if (nread == 0) {
			break;
		}

		nleft -= nread;
		buf += nread;
	}

	return(nbytes - nleft);
}

int serial_write(int fd, const char *vptr, int n)
{
    int  nleft;
    int nwritten;

    nleft = n;
    while(nleft > 0)
    {
		if((nwritten = write(fd, vptr, nleft)) <= 0)
        {
            if(nwritten < 0&&errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        vptr += nwritten;
    }
    return(n);
}

int main(int argc, char *argv[])
{
	int serial_fd = -1;
	int result;
	char buf[BUF_SIZE] = {0};
	char str[] = "hello test";

	serial_fd = serial_open("/dev/ttyH0");
	if(serial_fd < 0) {
		printf("serial_open error!\n");
		exit(1);
	}

	serial_set(serial_fd, 115200, 8, 'O', 1);

	while(1) {
		serial_write(serial_fd, str, strlen(str));
		usleep(100000);
	}

	while (1) {
		memset(buf, 0, sizeof(buf));
		result = treadn(serial_fd, buf, 512, 0, 100);
		if (strlen(buf) > 0)
			printf("buf=%s\n", buf);
	//	if (result < -1) {
	//		printf("handle_read error!\n");
	//	}
	}

	close(serial_fd);
	return 0;
}
