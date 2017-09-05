#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stddef.h>


#define MAX_READ_TIMEOUT 8000
#define BUFSIZE 1024
static const char *sock_path="/home/jihj/Mystudy/test/scan_cmd.sock";

static void error(char *msg) {
	perror(msg);
}

int main(int argc, char **argv) {
	int sockfd, n;
	int serverlen, len;
	struct sockaddr_un serveraddr;
	char buf[2048]={0};
	int ret;
	fd_set rdfds;
	struct timeval tv;

	if (argc != 2) {
		fprintf(stderr,"usage: %s <cmd>\n", argv[0]);
		exit(0);
	}
	len=strlen(argv[1]);
	strncpy(buf, argv[1], 1000);

	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	/*fill in socket address structure*/
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, sock_path);
	serverlen = offsetof(struct sockaddr_un, sun_path) + strlen(serveraddr.sun_path);

	ret = connect(sockfd, (struct sockaddr*)&serveraddr, serverlen);
	if (ret < 0) {
		error("ERROR in connect");
		printf("Error\n");
		goto exit;
	}


	n = send(sockfd, buf, len+1 /* plus zero */, 0);
	if (n < 0) {
		error("ERROR in sendto");
		printf("Error\n");
		goto exit;
	}

	FD_ZERO(&rdfds);
	FD_SET(sockfd, &rdfds);
	tv.tv_sec = MAX_READ_TIMEOUT/1000;
	tv.tv_usec = MAX_READ_TIMEOUT%1000;

	ret = select(sockfd+1, &rdfds, NULL, NULL, &tv);

	if (ret > 0 && FD_ISSET(sockfd, &rdfds)) {
		memset(buf, 0, sizeof(buf));
		usleep(10000); /* sleep 0.01 second for wait all data */
		n = recv(sockfd, buf, sizeof(buf), 0);
		if (n < 0) {
			//error("ERROR in recvfrom");
			goto exit;
		}
		printf("ddd=%s\n", buf);
	} else {
		printf("Timeout\n");
		goto exit;
	}

exit:
	if (sockfd > 0)
		close(sockfd);
	return 0;
}

