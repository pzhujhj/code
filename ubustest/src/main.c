#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>

#include <libubox/uloop.h>
#include <libubox/kvlist.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include <linux/i2c.h>

#include "include/test.h"
#include "include/debug.h"
#include "include/ubus.h"

#define PI 3.14159265358979323846

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define _unused __attribute__((unused))

struct uloop_timeout sensor_iic_timer;

struct blob_buf b;

static SENSOR_MSG sensor_msg;
static int sensor_fd = -1;

static void usage()
{
	fprintf(stderr,
			"\n"
			"usage: netstatsd [-hdv]"
			"\n"
			"options:\n"
			"    -h  show this usage\n"
			"    -d  device [device=/dev/i2c-x x=0,1,2...]\n"
			"    -v  show more debug messages (-vv for even more)\n");
	exit(0);
}

SENSOR_MSG *get_sensor_value() 
{
	strcpy(sensor_msg.xAccl, "aaa");
	strcpy(sensor_msg.yAccl, "bbb");

	return &sensor_msg;
}

int main (int argc, char *argv[])
{
	int opt;

	const char *device = NULL;

	while ((opt = getopt(argc, argv, "d:h:v")) != -1) {
		switch (opt) {
		case 'd':
			device = optarg;
			break;
		case 'v':
			set_debug_level(get_debug_level()-1);
			break;
		case 'h':
			usage();
			break;
		default:
			usage();
			break;
		}
	}

	if (device == NULL)
		usage(argv[0]);

	uloop_init();
	if (service_init() != SUCCESS) {
		printf("service fail\n");
		return -1;
	}
	uloop_run();
	uloop_done();

	close(sensor_fd);

	return 0;
}
