#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#if 0
static int system_cmd(char *fmt, ...)
{
	char p[256];
	va_list ap;
	int pid = fork();
	if(!pid)
	{
		va_start(ap, fmt);
		vsnprintf(p, 256, fmt, ap);
		va_end(ap);
		execlp("/bin/sh", "/bin/sh", "-c", p, NULL);
		syslog(0, "oooppps, called %s which does not exist\n", p);
		exit(0);
	} else {
		int t;
		do {
			t = waitpid(pid, NULL, 0);
		} while(t <= 0);
	}
	return 0;
}
#else
static int system_cmd(char *fmt, ...)
{
	char p[256];
	va_list ap;
	int ret;
	va_start(ap, fmt);
	vsnprintf(p, 256, fmt, ap);
	va_end(ap);
	ret = system(p);
	return ret;
}
#endif

int main()
{
	//system_cmd("echo %d %s>> /test/1.txt", 2, "cccc");	
	system_cmd("cat /test/1.txt");	
	
	return 0;
}
