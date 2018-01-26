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
#include "message.h"

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

static int send_client_msg(int clifd)
{
	int readn;
	char rcvbuf[BUFSIZE] = {0}, sendbuf[BUFSIZE] = {0};
	MSGSTR *msgstr;

	memset(rcvbuf, 0, BUFSIZE);
	readn = read(clifd, rcvbuf, BUFSIZE);
	debug(MSG_CRIT, "rcvbuf=%s", rcvbuf);

#if 0  //method one
	msgstr  = (MSGSTR *)malloc(sizeof(MSGSTR));
	msgstr->number = 1;
	strcpy(msgstr->value, "hello");
	write(clifd, msgstr, sizeof(MSGSTR));
	free(msgstr);
#endif

#if 0 //method two error
	int size = 128;
	char buf[BUFSIZE] = {0};
	msgstr  = (MSGSTR *)malloc(sizeof(MSGSTR));
	msgstr->number = 2;
	msgstr->value = malloc(sizeof(char)*size);
	//msgstr->value = ((char *)msgstr) + sizeof(MSGSTR);
	strcpy(msgstr->value, "wwwwww");
	memcpy(buf, msgstr, BUFSIZE);
	//write(clifd, msgstr, sizeof(MSGSTR));
	printf("%p %p\n", &msgstr->number, msgstr->value);
	write(clifd, buf, BUFSIZE);
	free(msgstr->value);
	free(msgstr);
#endif

#if 1 //method three
	int mlen;
	char buf[] = "fdfdfd";
	mlen = strlen(buf);
	msgstr  = (MSGSTR *)malloc(sizeof(MSGSTR) + sizeof(char)*mlen);
	msgstr->type = htonl(3);
	msgstr->len = htonl(mlen);
	strncpy(msgstr->value, buf, mlen);
	printf("%p %p %p\n", &msgstr->type, &msgstr->len, msgstr->value);
	write(clifd, msgstr, sizeof(MSGSTR) + sizeof(char)*mlen);
	free(msgstr);
#endif

	return 1;
}

int main(int argc, char **argv)
{
	int clifd, ret;
	struct sockaddr_in addr;
	int time = 5;

	if (argc < 5)
		print_help(argv);

	if ((clifd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		debug(MSG_ERROR, "socket error!");
		exit(1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[4]));
	if (inet_pton(AF_INET, argv[2], &addr.sin_addr.s_addr) <= 0) {
		debug(MSG_ERROR, "servip invalid!");
		exit(1);
	}

	connect(clifd, (struct sockaddr *)&addr, sizeof(addr));
	//str_cli(stdin, clifd);

	do {
		ret = send_client_msg(clifd);
	}while((ret == 1) && (time--));
	
	close(clifd);
	return 0;
}

