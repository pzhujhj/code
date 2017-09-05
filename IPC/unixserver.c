#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/un.h>
#include <pthread.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>

#define SOCK_UNIX_FILE "/home/jihj/Mystudy/test/scan_cmd.sock"
#define BUF_SIZE 32

static struct sockaddr_un server_address;

static int create_unix_server()
{
	int server_fd;
	int server_len;
		
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	unlink(SOCK_UNIX_FILE); //加上这个是避免出现bind:Address already in use错误，AF_UNIX对setsockopt不起作用

	/*fill in socket address structure*/
	memset(&server_address, 0, sizeof(server_address));
	server_address.sun_family = AF_UNIX;
	strcpy(server_address.sun_path, SOCK_UNIX_FILE);
	/*offsetof：计算sun_path从结构开始处的偏移量*/
	server_len = offsetof(struct sockaddr_un, sun_path) + strlen(server_address.sun_path);

	if(bind(server_fd, (struct sockaddr *)&server_address, server_len) < 0) {
		printf("fail to bind:%s\n", strerror(errno));
		goto errout;
	}

	if(listen(server_fd, 1) < 0) {
		printf("fail to listen:%s\n", strerror(errno));
		goto errout;
	}

	return server_fd;

errout:
	close(server_fd);
	unlink(SOCK_UNIX_FILE);
	return -1;
}

static void read_sn(int cli_fd)
{
	printf(" read_sn\n");
	
	write(cli_fd, "test", 4);
}

int main(int argc, char *argv[])
{
	int server_sockfd, client_sockfd;
	socklen_t client_len;
	char recv_buf[BUF_SIZE] = {0};
	char ch;

	struct sockaddr_un client_address;

	unlink(SOCK_UNIX_FILE);
	server_sockfd = create_unix_server();
	if(server_sockfd < 0)
		exit(EXIT_FAILURE);
	
	client_len = sizeof(client_address);
	while (1) {
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&server_address, &client_len);
		if(client_sockfd < 0) {
			printf("accept error!\n");
			unlink(SOCK_UNIX_FILE);
			exit(EXIT_FAILURE);
		}

		memset(recv_buf, 0, BUF_SIZE);
		read(client_sockfd, recv_buf, BUF_SIZE);
		ch = recv_buf[0];

		switch (ch) {
			case 'S':
				read_sn(client_sockfd);
				break;
			default:
				printf("default\n");
				break;
		}
		
		close(client_sockfd);
	}

	close(server_sockfd);
	unlink(SOCK_UNIX_FILE);
	return 0;
}
