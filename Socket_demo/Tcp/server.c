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
#include "message.h"

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

#if 0
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
#endif

static int readXbytes(int fd, unsigned int x, void* buffer)
{
	int bytesread = 0;
	int readn;

	while (bytesread < x) {
		readn = read(fd, buffer + bytesread, x - bytesread);
		if (readn == 0) {
			debug(MSG_ERROR, "client closed this connection!");
			return 0;
		} else if (readn < 0) {
			debug(MSG_ERROR,"Error, read socket");
			return -1;
		}

		bytesread += readn;
	}
	return x;
}

static int handle_request(int fd)
{
	int ret, readn;
	MSGSTR *rmsgstr;

#if 0 //methmod one
	/*这里用到结构体直接传送数据，也可以将结构体转换成数组进行传送数据*/
	rmsgstr = (MSGSTR *)malloc(sizeof(MSGSTR));
	readn = read(fd, rmsgstr, sizeof(MSGSTR));
	if(readn < 0)
		debug(MSG_ERROR, "read socket error");

	printf("%p %p\n", &rmsgstr->number, rmsgstr->value);
	debug(MSG_CRIT, "recv byte:%d msg:%d %s", readn, rmsgstr->number, rmsgstr->value);
	free(rmsgstr);
#endif

#if 0 //method two error
	int size = 128;
	char buf[BUFSIZE] = {0};
	readn = read(fd, buf, BUFSIZE);
	printf("buf=%s\n", buf);
	rmsgstr = (MSGSTR *)malloc(sizeof(MSGSTR));
	memcpy(rmsgstr, buf, sizeof(rmsgstr));
	rmsgstr->value = malloc(sizeof(char)*size);
	//readn = read(fd, rmsgstr, sizeof(MSGSTR));
	//if(readn < 0)
	//	debug(MSG_ERROR, "read socket error");

	printf("%p %p\n", &rmsgstr->number, rmsgstr->value);

	debug(MSG_CRIT, "recv byte:%d msg:%d %s", readn, rmsgstr->number, rmsgstr->value);
	
	free(rmsgstr->value);
	free(rmsgstr);	
#endif

#if 1 //method three
	unsigned int type;
	unsigned int len;
	if((ret = readXbytes(fd, sizeof(type), (void *)&type)) <= 0) {
		debug(MSG_ERROR, "read type error!");
		return ret;
	}
	type = ntohl(type);

	if((ret = readXbytes(fd, sizeof(len), (void *)&len)) <= 0) {
		debug(MSG_ERROR, "read type error!");
		return ret;
	}
	len = ntohl(len);

	rmsgstr = (MSGSTR *)malloc(sizeof(MSGSTR) + sizeof(char)*len);
	rmsgstr->type = type;
	rmsgstr->len = len;
	
	if (len > 0 && (ret = readXbytes(fd, rmsgstr->len, (void *)(rmsgstr->value))) <= 0) {
		free(rmsgstr);
		return ret;
	}	

	printf("%p %p %p\n", &rmsgstr->type, &rmsgstr->len, rmsgstr->value);
	debug(MSG_CRIT, "recv msg:%d %d %s", rmsgstr->type, rmsgstr->len, rmsgstr->value);

	free(rmsgstr);

#endif
	return 1;
}

int main(int argc, char **argv)
{
	int sockfd, connfd, maxfd, maxi, i, readfd;
	socklen_t clilen;
	//pid_t childpid;
	struct sockaddr_in cliaddr;
	int client[MAX_CLIENT], nready, n;
	fd_set rset, allset;
	char buf[BUFSIZE] = {0};
	MSGSTR *smsgstr;

	if (argc > 1)
		parameter_parser(argc, argv);

	if((sockfd = open_tcp_socket(servip, servport)) < 0) {
		debug(MSG_ERROR, "open socket error!");
		exit(1);
	}

#if 0
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
#endif

	maxfd = sockfd;
	maxi = -1; //index into client[] array

	for(i = 0; i < MAX_CLIENT; i++) 
		client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);

	for(;;) {
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);//返回成功表示文件描述符状态该改变的的个数
		if (nready < 0) {
			debug(MSG_ERROR, "select error!");
			exit(1);
		}

		if(FD_ISSET(sockfd, &rset)) {//new client connection
			clilen = sizeof(cliaddr);
			if((connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen)) < 0) {
				debug(MSG_ERROR, "accept error!");
				exit(1);
			}

			debug(MSG_CRIT, "new client: %s, port %d", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

			for(i = 0; i < MAX_CLIENT; i++) {
				if(client[i] < 0) {
					client[i] = connfd; //save descriptor
					break;
				}				
			}

			if(i == MAX_CLIENT) {
				debug(MSG_ERROR, "too many clients!");
				continue;
			}

			FD_SET(connfd, &allset); //add new descriptor to set
			if(connfd > maxfd)
				maxfd = connfd;

			if(i > maxi)
				maxi = i;

			//send message to new client
			memset(buf, 0, BUFSIZE);
			snprintf(buf, BUFSIZE, "you id:%d", i);
			write(connfd, buf, strlen(buf));

			if (--nready <= 0)
				continue; // no more readable descriptors

		}

		memset(buf, 0, sizeof(buf));
		for (i = 0; i <= maxi; i++) {//check all clients for data
			if ((readfd = client[i]) < 0)
				continue;
			if(FD_ISSET(readfd, &rset)) {
				#if 0
				if((n = read(readfd, buf, 512)) == 0) {
					//connection closed by client
					close(readfd);
					FD_CLR(readfd, &allset);
					client[i] = -1;
				}else 
					write(readfd, buf, n);
				#endif

				if(handle_request(readfd) <= 0) {
					FD_CLR(readfd, &allset);
					close(readfd);
					client[i] = -1;
				}

				if(--nready <= 0)
					break;				
			}
		}
	}

	close(sockfd);
	return 0;
}
