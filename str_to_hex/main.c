#include<stdio.h>
#include <string.h>

/*将字符串转换成十六进制*/
static char valueToHex(const int value)
{
	char ret = '\0';

	if(value >= 0 && value <= 9) {
		ret = (char)(value + 48); //48为ascii编码的‘0’字符编码值
	}else if(value >= 10 && value <= 15) {
		ret = (char)(value - 10 + 65); //减去10则找出其在16进制的偏移量，65为ascii的'A'的字符编码值
	}

	return ret;
}

/******************************************************* 
* Function Name:  strTohex 
* Purpose:    将字符串转换成十六进制 
* Params :  
*  @str 需要转换的字符串 
*  @hexc 存储转换成十六进制 
* Return:  
* Limitation:返回值解释说明 
*******************************************************/

static void strTohex(char *str, char *hexc)
{
	int hbyte, lbyte;
	int tmp;

	if(str == NULL || hexc == NULL)
		return;
	
	if (strlen(str) == 0) 
		return;

	printf("str=%s\n", str);

	while(*str) {
		tmp = (int)*str;
		hbyte = tmp >> 4;  //取高字节
		lbyte = tmp & 0x0f; //取低字节
		
		printf("hbyte=%d lbyte=%d\n", hbyte, lbyte);
		*hexc++ = valueToHex(hbyte);
		*hexc++ = valueToHex(lbyte);

		str++;
	}

	*hexc = '\0';
}

/*将十六进制转换成字符串*/
int hexTovalue(const char ch)
{
	int ret = 0;

	if(ch >= '0' && ch <= '9')
		ret = (int)(ch - '0');
	else if(ch >= 'A' && ch <= 'F')
		ret = (int)(ch - 'A') + 10;
	else if(ch >= 'a' && ch <= 'f')
		ret = (int)(ch - 'a') + 10;
	else 
		ret = -1;

	return ret;	
}

int hexTostr(char *hexc, char *str)
{
	int tmp = 0;
	int hbyte, lbyte;

	if (hexc == NULL || str == NULL) 
		return -1;
	
	if(strlen(hexc)%2 == 1) //判断16进制字符串是否正确，只能为偶数
		return -1;

	while(*hexc) {
		hbyte = hexTovalue(*hexc);
		if(hbyte < 0) {
			*str = '\0';
			return -1;
		}
		
		hexc++; //指针移动到下一个字符上
	
		lbyte = hexTovalue(*hexc);
		if(lbyte < 0) {
			*str = '\0';
			return -1;
		}

		tmp = hbyte << 4 | lbyte;
		*str++ = (char)tmp;

		hexc++;
	}

	*str = '\0';
	return 0;
}

int main()
{
	//char msg[] = "hell34w";
	char msg[] = "h";
	char hex[128] = {0};
	char str[128] = {0};

	memset(hex, 0, sizeof(hex));
	memset(str, 0, sizeof(str));

	strTohex(msg, hex);
	printf("hex:%s\n", hex);
	hexTostr(hex, str);
	printf("str:%s\n", str);
	return 0;
}
