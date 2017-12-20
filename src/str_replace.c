#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char *str_replace(const char *src, const char *matchstr, const char *replacestr)
{
	char *temp = NULL;
	const char *match = NULL;
	int count = 0;

	int size = strlen(src) + 1;
	int matchstrlen = strlen(matchstr);
	int replacestrlen = strlen(replacestr);
	char *value = malloc(size);
	char *dst = value; //dst作为操作指针,value作为存放数据的指针

	if((NULL == matchstr) || (NULL == replacestr))
		return NULL;

	if ( value != NULL ) {
		for ( ;; ) {
			match = strstr(src, matchstr); //查找是否包含需要替换的字符
			if ( match != NULL ) {    //查找到匹配的字符
				count = match - src; //得到需要替换字符的地址,也表示src和match之间有count个char变量的空间 
				printf("count=%d\n", count);
				size += replacestrlen - matchstrlen; //重新计算新的字符串总长度
				/*realloc用法参考:http://c.biancheng.net/cpp/html/2536.html*/
				temp = realloc(value, size); //重新申请分配地址
				if ( temp == NULL ) {
					free(value);
					return NULL;
				}
				
				dst = temp + (dst - value);
				printf("dst=%s\n", dst);
				value = temp;
				printf("value=%s\n", value);
				memmove(dst, src, count); //将匹配字符之前的字符保存到dst中
				printf("--1-dst=%s\n", dst);

				src += count; //将原字符串首地址移到匹配的字符位置
				dst += count; //将dst中存放数据的首地址向后移count
				printf("src=%s\n", src);

				printf("--2-dst=%s\n", dst);
				memmove(dst, replacestr, replacestrlen); //将要替换的字符拼接到dst中
				printf("--3-dst=%s\n", dst);

				src += matchstrlen; //将原字符串首地址向后移动,跳过要替换的字符串
				dst += replacestrlen; //dst地址继续向后移动
			}
			else { /* No match found. */
				strcpy(dst, src);
				break;
			}
		}
	}

	return value;
}

int main()
{
	char *data;
	char buf[] = "-1112r2223,445";
	//for(;;) {
		data = str_replace(buf,"-",":");	
		printf("%s\n",data);
		free(data);
	//}
	return 0;
}
