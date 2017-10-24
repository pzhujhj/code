#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "debug.h"

#define MAX(x,y) (x)>(y)?(x):(y)

static void print_help(char **argv)
{
	printf("%s usage:\n"
			"\t-s, --servip <ip addr>\t set server ip address\n"
			"\t-p, --port   <port>   \t set server port\n"
			"\t-h, --help            \t show this message and quit\n",
			argv[0]);
	exit(0);
}

static void str_cli(FILE *fp, int clifd)
{
	char sendline[512], recvline[512];
	int maxfd;
	fd_set rset;

	FD_ZERO(&rset);

	for(;;) {
		FD_SET(fileno(fp), &rset);
		FD_SET(clifd, &rset);

		maxfd = MAX(fileno(fp), clifd) + 1;
		select(maxfd, &rset, NULL, NULL, NULL);

		if(FD_ISSET(clifd, &rset)) {
			if(read(clifd, recvline, 512) == 0)
				printf("recv over!\n");
			fputs(recvline, stdout);
		}

		if(FD_ISSET(fileno(fp), &rset)) {
			if(fgets(sendline, 512, fp) == NULL)
				return;
			write(clifd, sendline, 512);
		}
	}

}

int main(int argc, char **argv)
{
	int clifd;
	struct sockaddr_in addr;

	if (argc < 2)
		print_help(argv);

	if ((clifd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		debug(MSG_ERROR, "socket error!");
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr) <= 0) {
		debug(MSG_ERROR, "servip invalid!");
		exit(1);
	}

	connect(clifd, (struct sockaddr *)&addr, sizeof(addr));
	str_cli(stdin, clifd);
	
	close(clifd);
	return 0;
}

