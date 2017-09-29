#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/***************************************
*功能：实现十进制数转换成任意进制的数
*
*原理：
*24转换成十六进制为18-->就是24/16--1 24%16=8
*
***************************************/

static int HEX;

void showhex(int n)
{
    if(n>(HEX-1))
        showhex(n/M);

	if (HEX == 16) {
		switch (n%HEX)
		{
			case 10:
				printf("%c", 'a');
				break;
			case 11:
				printf("%c", 'b');
				break;
			case 12:
				printf("%c", 'c');
				break;
			case 13:
				printf("%c", 'd');
				break;
			case 14:
				printf("%c", 'e');
				break;
			case 15:
				printf("%c", 'f');
				break;
			default:
				printf("%d", n%HEX);	
		}

	}else
    	printf("%d",n%HEX);
}

int main(int argc, char *argv[])
{
    int n;
	
    if(argc < 2)
    {
        printf("please input two parament\n");
        sleep(1);
        return 0;
    }
	
    HEX = atoi(argv[1]);
    while(1)
    {
        printf("\rplease input Decimal number:");
        scanf("%d",&n);
        showhex(n);
        printf("\n");
    }
	
    return 0;
}

