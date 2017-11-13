#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	struct in_addr start_netaddr, lanIp, lanMask, start_addr;
	struct in_addr tmp_addr;
	unsigned int addr_tmp = 0;
	unsigned int lanIpVal, lanMaskVal;
	int ret;

	addr_tmp = 0x00;
	ret = inet_aton("192.168.34.100", &start_addr);
	printf("ret=%d\n", ret);
	memcpy(&addr_tmp, &start_addr, 4);
	inet_aton("192.168.3.1", &lanIp);
	inet_aton("255.255.255.0", &lanMask);
	printf("%d 0x%x\n", addr_tmp, addr_tmp);

	memcpy(&lanIpVal, &lanIp, 4);
	memcpy(&lanMaskVal, &lanMask, 4);

	start_netaddr.s_addr = (addr_tmp & 0xff000000) | (lanIpVal & lanMaskVal);
	printf("%d %s\n", (addr_tmp & 0xff000000), inet_ntoa(start_netaddr));

	printf("0x%x\n", lanIpVal);
	tmp_addr.s_addr = (addr_tmp & 0xff0000) | (lanIpVal & 0xff00ffff); 
	printf("%x %s\n", (addr_tmp & 0xff0000), inet_ntoa(tmp_addr));
	return 0;
}
