#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <syslog.h>
#include <stdarg.h>
#include <uci.h>
#include <math.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <libubox/uloop.h>
#include <libubox/kvlist.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>

#define LOG_TAG "UCITEST"
#define UCI_SUCCESS 0
#define UCI_FAIL 1

static void uci_context_destroy(struct uci_context *ctx)
{
	if (ctx)
		uci_free_context(ctx);
}

static int uci_revert_state(struct uci_context *c, char *str)
{
	struct uci_ptr ptr;

	if (uci_lookup_ptr(c, &ptr, str, false) != UCI_OK){
		uci_perror(c, LOG_TAG);
		return -UCI_FAIL;
	}

	if (uci_revert(c, &ptr) != UCI_OK) {
		uci_perror(c, LOG_TAG);
		return -UCI_FAIL;
	}

	if (uci_save(c, ptr.p) != UCI_OK) {
		uci_perror(c, LOG_TAG);
		return -UCI_FAIL;
	}

	return UCI_SUCCESS;
}

static int uci_set_state(struct uci_context *c, char *str)
{
	struct uci_ptr ptr;

	if (uci_lookup_ptr(c, &ptr, str, false) != UCI_OK){
		uci_perror(c, LOG_TAG);
		return -UCI_FAIL;
	}

	if (uci_set(c, &ptr) != UCI_OK) {
		uci_perror(c, LOG_TAG);
		return -UCI_FAIL;
	}

	if (uci_save(c, ptr.p) != UCI_OK) {
		uci_perror(c, LOG_TAG);
		return -UCI_FAIL;
	}

	return UCI_SUCCESS;
}

static struct uci_context *uci_context_init()
{
	struct uci_context *c;

	c = uci_alloc_context();
	if (c == NULL) {
		printf("uci out of memory\n");
		return NULL;
	}

	uci_set_savedir(c, "/var/state");

	return c;
}

static int uci_save_data(struct uci_context *ctx, char *content, char *value)
{
	char str[64] = {0};
	
	memset(str, 0, sizeof(str));

	snprintf(str, sizeof(str), "%s", content);
	if (uci_revert_state(ctx, str) < 0)
		return -UCI_FAIL;

	snprintf(str, sizeof(str), "%s=%s", content, value);
	if (uci_set_state(ctx, str) < 0)
		return -UCI_FAIL;

	return UCI_SUCCESS;
}

static const char *uci_get_option(const char *p, const char *s, const char *o)
{
	struct uci_context *ctx = NULL;
	struct uci_element *e   = NULL;
	const char *value = NULL;
	struct uci_ptr ptr;

	ctx = uci_context_init();
	if (!ctx) goto out;

	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.package = p;
	ptr.section = s;
	ptr.option  = o;

	if (uci_lookup_ptr(ctx, &ptr, NULL, true) != UCI_OK) {
		printf("uci lookup ptr%s.%s.%s error", p, s, o);
		goto out;
	}
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		goto out;
	}

	e = ptr.last;

	if (e && e->type==UCI_TYPE_OPTION && ptr.o->type==UCI_TYPE_STRING) {
		value = ptr.o->v.string;
	}
out:
	uci_context_destroy(ctx);

	return value;
}

static const char *uci_get_option_string(const char *p, const char *s, const char *o)
{
	return uci_get_option(p, s, o);
}

static const char *uci_get_section(const char *p, const char *s)
{
	struct uci_context *ctx = NULL;
	struct uci_section *sec = NULL;
	const char *value = NULL;
	struct uci_ptr ptr;

	ctx = uci_context_init();
	if (!ctx) goto out;

	memset(&ptr, 0, sizeof(struct uci_ptr));
	ptr.package = p;
	ptr.section = s;
	ptr.option  = NULL;

	if (uci_lookup_ptr(ctx, &ptr, NULL, true) != UCI_OK) {
		printf("uci lookup ptr%s.%serror", p, s);
		goto out;
	}

	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		goto out;
	}

	sec = ptr.s;

	if (sec)
		value = sec->type;
out:
	uci_context_destroy(ctx);

	return value;
}

static const char *uci_get_section_string(const char *p, const char *s)
{
	return uci_get_section(p, s);
}

static void usage(void)
{
	fprintf(stderr,
			"option: \n"
			"       -h show this usage\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int opt;
	struct uci_context *ctx = NULL;
	const char *value = NULL;
	const char *tmp = NULL;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
			case 'h':
				usage();
				break;
			default:
				usage();
				break;
		}
	}

	ctx = uci_context_init();
	if (ctx == NULL) {
		printf("uci_context_init error!\n");
		return -1;
	}

	if (uci_save_data(ctx, "myuci.enable", "aaaa") < 0) {
		printf("uci_save_data fail\n");
		goto out;
	}

	if (uci_save_data(ctx, "myuci.test.id", "5") < 0) {
		printf("uci_save_data fail\n");
		goto out;
	}

	if (uci_save_data(ctx, "myuci.ret.id", "5") < 0) {
		printf("uci_save_data fail\n");
		goto out;
	}

	value = uci_get_option_string("myuci", "test", "id");
	printf("value=%s\n", value);
	tmp = uci_get_section_string("myuci", "enable");
	printf("tmp=%s\n", tmp);

out:
	uci_context_destroy(ctx);

	return 0;
}
