#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

/*一个int型变量占4个字节,32位，是按照二进制存储的，也可以是十六进制表达方式
 *下面这个函数是将int型变量每一个字节存储在char型的buf中
 * 字符串只能是纯数字字符串
 * */
//方法一
static void num_to_hex_array1(int code)
{
	unsigned char buf[4] = {0};

	memset(buf, 0, sizeof(buf));

	//code = htonl(code);

	buf[0] = code >> 24 & 0xff;
	buf[1] = code >> 16 & 0xff;
	buf[2] = code >> 8 & 0xff;
	buf[3] = code & 0xff;
	printf("%02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
}

//方法二
static void num_to_hex_array2(int code, unsigned char *buf)
{
	char *p = (char *)&code;

	code = htonl(code);
	memcpy(buf, p, 4);
}

static void get_hex_num(unsigned char *buf)
{
	int i, ret = 0;
	char code[12] = {0};

#if 1
	for(i = 0; i < 4; i++) {
		if(i > 0)
			ret = ret << 8;
		ret += buf[i];
	}
#else
	ret = buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]; 
#endif
	memset(code, 0, 12);
	printf("ret=%d %x\n", ret, ret);
	sprintf(code, "%d", ret);
	printf("code=%s\n",code);
}

/*如果只取int型变量中的三个字节，方法如下*/
//方法一
static void num_to_hex_array3(int code)
{
	unsigned char buf[3] = {0};

	memset(buf, 0, sizeof(buf));

	//code = htonl(code);

	buf[0] = code >> 16 & 0xff;
	buf[1] = code >> 8 & 0xff;
	buf[2] = code & 0xff;

	printf("====%02x %02x %02x \n", buf[0], buf[1], buf[2]);
}
//方法二
static void num_to_hex_array4(int code, unsigned char *buf)
{
	char *p = (char *)&code;

	code = htonl(code);
	memcpy(buf, p+1, 3);
}

int main()
{

	char *str = "30169";
	//char *str = "20130169";
	int num;
	unsigned char msg[4] = {0};
	unsigned char msg1[3] = {0};

	num = atoi(str);
	printf("num=%d\n", num);

	memset(msg, 0, sizeof(msg));
	num_to_hex_array1(num);
	num_to_hex_array2(num, msg);
	printf("msg=%02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
	printf("msg=%s\n", msg);
	get_hex_num(msg);


	num_to_hex_array3(num);
	num_to_hex_array4(num, msg1);
	printf("msg1=%02x %02x %02x\n", msg1[0], msg1[1], msg1[2]);

	return 0;
}
