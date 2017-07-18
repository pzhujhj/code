#include <stdio.h>
#include <string.h>
#include <errno.h>

static int popen_cmd(char *cmd, char *str, int size)
{
	FILE *fp = NULL;

	fp = popen(cmd, "r");
	if(fp < 0) {
		printf("popen error:%s\n", strerror(errno));
		return -1;
	}

	while(fgets(str, size, fp)) {
		/*去掉返回值换行符*/
		if ('\n' == str[strlen(str)-1])
			str[strlen(str)-1] = '\0';
	}

	pclose(fp);
	return 0;
}


int main(int argc, char *argv[])
{
	char buf[128] = {0};

	/"调用系统命令的时候,添加上2>&1才会把错误信息输入到buf中"/
	popen_cmd("ls -lh 5.txt 2>&1", buf, sizeof(buf));
	printf("buf=%s\n", buf);
	return 0;
}
