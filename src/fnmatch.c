#include <fnmatch.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

/*****************************************************
*功能：实现模糊匹配查询
*使用函数api：
*函数原型
*DIR* opendir (const char * path ); （获取path子目录下的所由文件和目录的列表，如果path是个文件则返回值为NULL)
*struct dirent* readdir(DIR* dir_handle);功能:读取opendir 返回值的那个列表
*
*fnmatch：int fnmatch(const char *pattern, const char *string, int flags);
*FNM_NOESCAPE
*如果这个标志设置了，处理反斜杠为普通字符，而不是转义字符。
*FNM_PATHNAME
*如果这个标志设置了，string 里的斜杠只匹配 pattern 里的斜杠，它不能匹配星号(*)或问号(?)元字符，也不能匹配包含斜杠的中括号表达式([])。
*FNM_PERIOD
*如果这个标志设置了，string 里的起始点号必须匹配 pattern 里的点号。一个点号被认为是起始点号，如果它是string 第一个字符，或者如果同时设置了 FNM_PATHNAME，紧跟在斜杠后面的点号。
*FNM_FILE_NAME
*这是 FNM_PATHNAME 的 GNU 同义语。
*FNM_LEADING_DIR
*如果这个标志(GNU 扩展)设置了，模式必须匹配跟随在斜杠之后的 string 的初始片断。这个标志主要是给 glibc 内部使用并且只在一定条件下实现
*FNM_CASEFOLD
*如果这个标志(GNU 扩展)设置了，模式匹配忽略大小写。
*返回值：0，string 匹配 pattern；FNM_NOMATCH，没有匹配；或者其它非零值，如果出错。
*****************************************************/

static void fn_match(char *key, char *path)
{
	char *pattern = NULL;
	int ret;
	DIR *dir;
	struct dirent *entry;

	dir = opendir(path);
	pattern = key;

	if(dir != NULL)
	{
	    while( (entry = readdir(dir)) != NULL)
		{
			ret = fnmatch(pattern, entry->d_name, FNM_PATHNAME|FNM_PERIOD);
			if(ret == 0)
	        	printf("%s\n", entry->d_name);
			else if(ret == FNM_NOMATCH)
	        	continue;
			else
				printf("error file=%s\n", entry->d_name);
	    }

    	closedir(dir);
  	}	
	

}

int main(int argc, char *argv[])
{
	if (argv[1] && argv[2])
		fn_match(argv[1], argv[2]);

	return 0;
}

