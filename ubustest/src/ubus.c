#include "include/ubus.h"
#include "include/debug.h"

typedef struct _service_s {
	service_init_t init;
	char *name;
}SERVICE_S;

static SERVICE_S *s = NULL;

static struct ubus_context *ubus_ctx = NULL;
static struct ubus_method *main_object_methods = NULL;
static int main_object_methods_size = 0;
static struct blob_buf buf;

static struct ubus_object_type *get_object_type()
{
	static struct ubus_object_type main_object_type;

	main_object_type.name = strdup("ubustest");
	main_object_type.id   = 0;
	main_object_type.methods = main_object_methods;
	main_object_type.n_methods = main_object_methods_size;

	return &main_object_type;
}

static struct ubus_object *get_main_object()
{
	static struct ubus_object main_object;

	main_object.name = strdup("ubustest");
	main_object.type = get_object_type();
	main_object.methods = main_object_methods;
	main_object.n_methods = main_object_methods_size;

	return &main_object;
}

static void ubus_connect_handler(struct ubus_context *ctx)
{
	ubus_ctx = ctx;

	ubus_add_uloop(ubus_ctx);
	ubus_add_object(ubus_ctx, get_main_object());
}
static struct ubus_auto_conn conn = { .cb = ubus_connect_handler };

void add_ubus_method(const char *name, ubus_handler_t handler,
		const struct blobmsg_policy *policy, int n_policy)
{
	if (main_object_methods_size <= 0) {
		main_object_methods_size = 1;
		main_object_methods = calloc(1, sizeof(struct ubus_method)*main_object_methods_size);
	} else {
		main_object_methods_size++;
		main_object_methods = realloc(main_object_methods, sizeof(struct ubus_method)*main_object_methods_size);
	}
	main_object_methods[main_object_methods_size-1].name = name;
	main_object_methods[main_object_methods_size-1].handler = handler;
	main_object_methods[main_object_methods_size-1].mask = 0;
	main_object_methods[main_object_methods_size-1].tags = 0;
	main_object_methods[main_object_methods_size-1].policy = policy;
	main_object_methods[main_object_methods_size-1].n_policy = n_policy;	
}

int ubus_reply(struct ubus_request_data * req, struct blob_attr * msg)
{
	if (!ubus_ctx) {
		return -1;
	}
	return ubus_send_reply(ubus_ctx, req, msg);
}

/********ubus get method*********/
enum {
	GET_ATTR_TYPE,
	GET_ATTR_MAX
};

static const SENSOR_MSG *ubus_get_carinfo()
{
	SENSOR_MSG *ser_info = NULL;

	ser_info = get_sensor_value();
	
	return ser_info;
}

static struct get_method_s get_methods[] = {
	{.method="value",	   .handler=ubus_get_carinfo},
};

static const struct blobmsg_policy get_attrs[GET_ATTR_MAX] = {
	[GET_ATTR_TYPE] = {.name="type", .type=BLOBMSG_TYPE_STRING},
};

static int ubus_method_get(struct ubus_context *ctx, struct ubus_object *obj,
					struct ubus_request_data *req,
					const char *method, struct blob_attr *msg)
{
	int i;
	const char *type = NULL;
	const char *result = "error";
	struct blob_attr *tb[GET_ATTR_MAX];
	const SENSOR_MSG *data = NULL;

	if (msg == NULL) {
		goto out;
	}

	if (blobmsg_parse(get_attrs, GET_ATTR_MAX, tb, blobmsg_data(msg), blobmsg_len(msg)) != 0) {
		debug(MSG_ERROR,"ubus_method_get parse msg error\n");
		goto out;
	}

	if (!tb[GET_ATTR_TYPE] || blobmsg_type(tb[GET_ATTR_TYPE]) != BLOBMSG_TYPE_STRING) {
		debug(MSG_ERROR,"ubus_method_get param:type error\n");
		goto out;
	}

	type = blobmsg_get_string(tb[GET_ATTR_TYPE]);
	if (type == NULL) {
		debug(MSG_ERROR,"ubus_method_get param:type is NULL\n");
		goto out;
	}

	for (i=0; i<ARRAY_SIZE(get_methods); i++) {
		if (!strcmp(get_methods[i].method, type)) {
			//result = get_methods[i].handler();
			data = get_methods[i].handler();
			break;
		}
	}
	
	if (i == ARRAY_SIZE(get_methods)) {
		result = "unkown";
	}

out:
	blob_buf_init(&buf, 0);
	if (data) {
		blobmsg_add_string(&buf, "x", data->xAccl);
		blobmsg_add_string(&buf, "y", data->yAccl);
		blobmsg_add_string(&buf, "z", data->zAccl);
	}else {
		blobmsg_add_string(&buf, "data", result);
	}
	return ubus_reply(req, buf.head);
}

/********ubus set method*********/				
enum {
	SET_ATTR_TYPE,
	SET_ATTR_DATA,
	SET_ATTR_MAX
};

static bool ubus_set_print(struct blob_attr *data)
{
	//TODO
	const char *value = NULL;

	if (!data) return false;

	value = blobmsg_get_string(data);
	if (!value || strlen(value)<=0) {
		return false;
	}

	printf("value=%s\n", value);
	return true;
}

static struct vdr_set_method_s set_methods[] = {
	{.method="print",	   .handler=ubus_set_print},
};

static const struct blobmsg_policy set_attrs[SET_ATTR_MAX] = {
	[SET_ATTR_TYPE] = { .name="type", .type=BLOBMSG_TYPE_STRING },
	[SET_ATTR_DATA] = { .name="data", .type=BLOBMSG_TYPE_STRING },
};

static int ubus_method_set(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req,
			const char *method, struct blob_attr *msg)
{
	int i = 0;
	bool ret = false;
	const char *type = NULL;
	struct blob_attr *tb[SET_ATTR_MAX];

	if (msg == NULL) {
		goto out;
	}
	blobmsg_parse(set_attrs, SET_ATTR_MAX, tb, blobmsg_data(msg), blobmsg_len(msg));
	if (!tb[SET_ATTR_TYPE] || blobmsg_type(tb[SET_ATTR_TYPE]) != BLOBMSG_TYPE_STRING) {
		debug(MSG_ERROR, "ubus_method_set param:type error\n");
		goto  out;
	}
	type = blobmsg_get_string(tb[SET_ATTR_TYPE]);
	if (type == NULL) {
		debug(MSG_ERROR, "ubus_method_set param:type is NULL\n");
		goto out;
	}
	for (i=0; i<ARRAY_SIZE(set_methods); i++) {
		if (!strcmp(set_methods[i].method, type)) {
			ret = set_methods[i].handler(tb[SET_ATTR_DATA]);
			break;
		}
	}
	if (i == ARRAY_SIZE(set_methods)) {
		debug(MSG_DEBUG, "unkown set type:%s\n", type);
	}
out:
	blob_buf_init(&buf, 0);
	blobmsg_add_u8(&buf, "result", ret);
	return ubus_reply(req, buf.head);
}

int service_init()
{
	if (s->init() != SUCCESS)
		return FAIL;

	free(s->name);
	free(s);
	return SUCCESS;
}

static int serv_ubus_init()
{
	ubus_auto_connect(&conn);
	return SUCCESS;
}

void register_service(char *name, service_init_t init)
{
	s = (SERVICE_S *)malloc(sizeof(SERVICE_S));
	s->init = init;
	s->name = strdup(name);
}

/*注册ubus相关操作*/
REG_SERVICE_INFO(102, serv_ubus, serv_ubus_init)
/*实现ubus的子接口*/
REG_UBUS_METHOD(get, ubus_method_get, get_attrs, GET_ATTR_MAX)
REG_UBUS_METHOD(set, ubus_method_set, set_attrs, SET_ATTR_MAX)

