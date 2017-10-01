#include <sys/types.h>  
#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h> 
#include <errno.h>

  
static int handle_ping(char *dst, int cnt)  
{  
    FILE *stream;  
    char recvBuf[16] = {0};  
    char cmdBuf[256] = {0};  
  
    if (NULL == dst || cnt <= 0)  
        goto out;

  	memset(cmdBuf, 0, 256);
    sprintf(cmdBuf, "ping %s -c %d -i 0.2 | grep time= | wc -l", dst, cnt);  
    stream = popen(cmdBuf, "r");
	if (stream < 0) {
			printf("popen error:%s\n", strerror(errno));
			goto out;
	}
	memset(recvBuf, 0, 16);
    fread(recvBuf, sizeof(char), sizeof(recvBuf)-1, stream);  
    pclose(stream);  
  
    if (atoi(recvBuf) > 0)  
    	return 0;  

 out:
    return -1;  
}  
  
int main(int argc, char *argv[])  
{  
    if (argc != 3)  
    {  
		fprintf(stderr, "./a.out ip count\n");
        return -1;  
    }  
  
    if (handle_ping(argv[1], atoi(argv[2])))  
        printf("Network is not up to %s\n", argv[1]);  
    else  
        printf("Network is up to %s\n", argv[1]);  
  
    return 0;  
}  

