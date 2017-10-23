#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
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

#define DEFAULT_SERVER_IP "0.0.0.0"
#define DEFAULT_SERVER_PORT 9999
#define MAX_CLIENT 100

char *servip = DEFAULT_SERVER_IP;
unsigned short servport = DEFAULT_SERVER_PORT;


static void print_help(char **argv)
{
	printf("%s usage:\n"
			"\t-s, --servip <ip addr>\t set server ip address\n"
			"\t-p, --port   <port>   \t set server port\n"
			"\t-h, --help            \t show this message and quit\n",
			argv[0]);
	exit(0);
}

static const struct option long_options[] = {
	{"servip", required_argument, 0, 's'},
	{"port", required_argument, 0, 'p'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};

static void parameter_parser(int argc,  char **argv)
{
	int c;
	while ((c = getopt_long(argc, argv, "s:p:h", long_options, NULL)) != -1) {
		switch (c) {
			case 's':
				servip = optarg;
				break;
			case 'p':
				servport = atoi(optarg);
				if (servport <= 1024) {
					printf("server port need larger than 1024!\n");
					exit(1);
				}
				break;
			case 'h':
			case '?':
				print_help(argv);
				break;
			default:
				print_help(argv);
		}
	}
	
}

static int open_tcp_socket(char *servip, unsigned short servport)
{
	int serverfd;
	int on = 1;
	struct sockaddr_in addr;

	if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		debug(MSG_ERROR, "open_socket failed!");
		goto err;
	}

	setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(&on) );

	//fill struct
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(servport);
	if (inet_pton(AF_INET, servip, &addr.sin_addr.s_addr) <= 0) {
		debug(MSG_ERROR, "servip invalid!");
		goto err;
	}

	if (bind(serverfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		debug(MSG_ERROR, "bind socket failed!");
		goto err;
	}	

	if (listen(serverfd, MAX_CLIENT) < 0) {
		debug(MSG_ERROR, "listen socket error!");
		goto err;
	}
	
	return serverfd;
err:
	close(serverfd);
	return -1;
}

static void str_echo(int sockfd)
{
	ssize_t n;
	char buf[512];

again:
	while((n = read(sockfd, buf, 512)) > 0)
		write(sockfd, buf, n);
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
		debug(MSG_ERROR, "str_echo:read error!");
}

static void sig_prof(int signo)
{
	pid_t pid;
	int stat;

	pid = wait(&stat);
	printf("child %d terminated\n", pid);
	return;
}
static void init_sigaction(void)
{
	struct sigaction act;

	act.sa_handler = sig_prof;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);

	sigaction(SIGCHLD, &act, NULL);
}

int main(int argc, char **argv)
{
	int sockfd, connfd;
	socklen_t clilen;
	pid_t childpid;
	struct sockaddr_in cliaddr;

	if (argc > 1)
		parameter_parser(argc, argv);

	if((sockfd = open_tcp_socket(servip, servport)) < 0) {
		debug(MSG_ERROR, "open socket error!");
		exit(1);
	}

	//处理client端输入EOF(ctrl+d)后导致服务端子进程成为僵尸进程,所以通过信号处理,但是这个处理方式不很好
	init_sigaction();

	for(; ;) {
		clilen = sizeof(cliaddr);

		if((connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
			debug(MSG_ERROR, "accept error!");
			exit(1);
		}

		if((childpid = fork()) == 0) {
			close(sockfd);
			str_echo(connfd);
			exit(0);
		}

		close(connfd);
	}

	close(sockfd);
	return 0;
}
