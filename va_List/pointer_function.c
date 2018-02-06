#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

/******************************************************* 
* File annotation: 这个文件主要是研究指针函数(char *)的 
*返回值问题
*
* 正确结果：-rw-rw-r--. 1 xxxx xxxx 9 7月  17 09:21 /home/xxxx/mygit/source_code/va_List/1.txt
* 错误结果：-rw-rw-r--. 1 xxxx xxxx 9 7月  17 09:21 /hoBy·~\·t/so
*
* 方法一：错误使用指针函数的返回值
* str数组是一个局部的数组，存放在栈上的，当popen_cmd运行完后会释放，所以dest=str相当于还是指向一个无知的地址
* 虽然编译没有报错，最终的输出也有结果，但是结果和实际结果有差别，那是因为这个无知的地址暂时没有被其它程序所
* 使用。这是一个比较常见的隐形错误，需要注意
*
*
* ******************************************************/
#if 0
/*方法一:*/
static char *popen_cmd(char *cmd)
{
	FILE *fp = NULL;
	char str[512] = {0};
	char *dest = NULL;

	fp = popen(cmd, "r");
	if(fp < 0) {
		printf("popen error:%s\n", strerror(errno));
		return NULL;
	}

	memset(str, '\0', sizeof(str));
	while(fgets(str, 512, fp)) {
		/*去掉返回值换行符*/
		if ('\n' == str[strlen(str)-1]) {
			str[strlen(str)-1] = '\0';
		}
	}
	dest = str;

	pclose(fp);
	return dest;
}

int main(int argc, char *argv[])
{
	char *p = NULL;

	p = popen_cmd("ls -lh /home/jihj/mygit/source_code/va_List/1.txt 2>&1");
	printf("p=%s\n", p);
	return 0;
}
#endif

#if 0
/******************************************************* 
 *方法二：正确使用
 *这个方法和方法一区别在于str是main函数中buf传递的地址，popen_cmd
 *执行完后,dest仍然是指向buf的地址的
 *
 *
*******************************************************/
static char *popen_cmd(char *cmd, char *str)
{
	FILE *fp = NULL;
	char *dest = NULL;

	fp = popen(cmd, "r");
	if(fp < 0) {
		printf("popen error:%s\n", strerror(errno));
		return NULL;
	}

	while(fgets(str, 512, fp)) {
		/*去掉返回值换行符*/
		if ('\n' == str[strlen(str)-1]) {
			str[strlen(str)-1] = '\0';
		}
	}

	dest = str;

	pclose(fp);
	return dest;
}

int main(int argc, char *argv[])
{
	char *p = NULL;
	char buf[512] = {0};

	p = popen_cmd("ls -lh /home/jihj/mygit/source_code/va_List/1.txt 2>&1", buf);
	printf("p=%s\n", p);
	return 0;
}
#endif

#if 0
/******************************************************* 
 *方法三：正确使用
 *用static修饰str后，存储的位置由栈区变为静态存储区
 *直到整个程序运行完才释放
*******************************************************/
static char *popen_cmd(char *cmd)
{
	FILE *fp = NULL;
	static char str[512] = {0};

	fp = popen(cmd, "r");
	if(fp < 0) {
		printf("popen error:%s\n", strerror(errno));
		return NULL;
	}

	memset(str, 0, sizeof(str));
	while(fgets(str, 512, fp)) {
		/*去掉返回值换行符*/
		if ('\n' == str[strlen(str)-1]) {
			str[strlen(str)-1] = '\0';
		}
	}

	pclose(fp);
	return str;
}

int main(int argc, char *argv[])
{
	char *p = NULL;

	p = popen_cmd("ls -lh /home/jihj/mygit/source_code/va_List/1.txt 2>&1");
	printf("p=%s\n", p);
	return 0;
}
#endif

#if 0
/******************************************************* 
 *方法四：正确使用
 *在函数体内进行malloc分配内存，然后释放可以
 *直到整个程序运行完才释放
 *这个方法代码的耦合度增大，调用函数时必须知道内部细节
*******************************************************/
static char *popen_cmd(char *cmd)
{
	FILE *fp = NULL;
	char *str = (char *)malloc(512);

	fp = popen(cmd, "r");
	if(fp < 0) {
		printf("popen error:%s\n", strerror(errno));
		return NULL;
	}

	while(fgets(str, 512, fp)) {
		/*去掉返回值换行符*/
		if ('\n' == str[strlen(str)-1]) {
			str[strlen(str)-1] = '\0';
		}
	}

	pclose(fp);
	return str;
}

int main(int argc, char *argv[])
{
	char *p = NULL;

	p = popen_cmd("cat  /home/jihj/gitcode/source_code/va_List/1.txt 2>&1");
	printf("p=%s\n", p);

	free(p); //这里一定要记住释放，防止内存泄露,其实这里的p所指向的地址就是str申请的堆地址,所以释放p就是释放str
	p = NULL;
	return 0;
}
#endif

/*******************************************************
 *方法五：正确使用
 *在函数体外进行malloc分配内存,这种代码耦合度更好
 *直到整个程序运行完才释放
*******************************************************/
static char *popen_cmd(char *cmd, char *str)
{
	FILE *fp = NULL;

	fp = popen(cmd, "r");
	if(fp < 0) {
		printf("popen error:%s\n", strerror(errno));
		return NULL;
	}

	while(fgets(str, 512, fp)) {
		/*去掉返回值换行符*/
		if ('\n' == str[strlen(str)-1]) {
			str[strlen(str)-1] = '\0';
		}
	}

	pclose(fp);
	return str;
}

int main(int argc, char *argv[])
{
	char *p;

	p = malloc(512);
	if(p == NULL) {
		printf("malloc error!\n");
		exit(1);
	}

	popen_cmd("cat  /home/jihj/gitcode/source_code/va_List/1.txt 2>&1", p);
	printf("p=%s\n", p);

	free(p);
	p = NULL;

	return 0;
}

