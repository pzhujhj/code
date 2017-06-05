#ifndef UBUS_H_
#define	UBUS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <endian.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/usock.h>
#include <libubox/ustream.h>
#include <libubus.h>

#include "test.h"

#define SUCCESS 1
#define FAIL 0

typedef int (*service_init_t)(void);

//typedef const char * (*get_method_t)();
typedef const SENSOR_MSG *(*get_method_t)();

struct get_method_s {
	char *method;
	get_method_t handler;
};

typedef bool (*set_method_t)(struct blob_attr *data);

struct vdr_set_method_s {
	char *method;
	set_method_t handler;
};


int service_init();
void add_ubus_method(const char *name, ubus_handler_t handler, const struct blobmsg_policy *policy, int n_policy);
int ubus_reply(struct ubus_request_data * req, struct blob_attr * msg);
void register_service(char *name, service_init_t init);

#define REG_SERVICE_INFO(priority, srv_name, srv_init) \
static void __attribute__((constructor(priority))) __reg_##srv_name##_service() \
{ \
	register_service(#srv_name, srv_init); \
}

#define REG_UBUS_METHOD(name, handler, policy, n_policy) \
static void __attribute((constructor)) __reg_ubus_method_##name() \
{ \
	add_ubus_method(#name, handler, policy, n_policy); \
}

#endif
