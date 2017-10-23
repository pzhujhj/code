#ifndef DEBUG_H_
#define DBBUG_H_

enum { MSG_INFO, MSG_DEBUG, MSG_WARNING, MSG_ERROR, MSG_CRIT };
enum { LOG_TYPE_SYSLOG, LOG_TYPE_STDOUT };

void debug(int level, char *fmt, ...);
int  get_debug_level();
void set_debug_level(int level);

#endif
