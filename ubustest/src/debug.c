#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>

#include "include/debug.h"
#include "include/test.h"

#define LOG_BUFFER_MAX_LEN 4096

static int log_type = LOG_TYPE_STDOUT;
static int debug_level = MSG_WARNING;
static int debug_timestamp = 0;

static void debug_print(int level, char *buf)
{
	if (level < debug_level)
		return ;

	assert(buf);

	if (log_type == LOG_TYPE_STDOUT) {
		printf("%s\n", buf);
		return ;
	}

	switch (level)
	{
	case MSG_INFO:
		syslog(LOG_INFO, "%s", buf);
		break;
	case MSG_DEBUG:
		syslog(LOG_DEBUG, "%s", buf);
		break;
	case MSG_WARNING:
		syslog(LOG_WARNING, "%s", buf);
		break;
	case MSG_ERROR:
		syslog(LOG_ERR, "%s", buf);
		break;
	case MSG_CRIT:
		syslog(LOG_CRIT, "%s", buf);
		break;
	default:
		syslog(LOG_DEBUG, "%s", buf);
	}
}


static void debug_get_timestamp(char *buf)
{
	struct timeval tv;

	if (!debug_timestamp)
		return ;

	gettimeofday(&tv, NULL);

	sprintf(buf, "%ld.%06u: ", (long) tv.tv_sec, (unsigned int) tv.tv_usec);
}

void debug(int level, char *fmt, ...)
{
	va_list ap;
	char    buf[LOG_BUFFER_MAX_LEN];

	if (level < debug_level)
		return ;

	memset(buf, 0, LOG_BUFFER_MAX_LEN);
	debug_get_timestamp(buf);

	va_start(ap, fmt);
//	vsprintf(buf+strlen(buf), fmt, ap);
	vsnprintf(buf+strlen(buf), LOG_BUFFER_MAX_LEN-strlen(buf)-1, fmt, ap);
	va_end(ap);

	debug_print(level, buf);
}

int get_debug_level()
{
	return debug_level;
}

void set_debug_level(int level)
{
	debug_level = level;
}
