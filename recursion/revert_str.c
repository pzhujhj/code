#include <stdio.h>
#include <string.h>

#if 0
static char *rever_str(char *str)     
{
	char temp;
	int i;
	
    if( !str )     
		return NULL;
    
    int len = strlen(str);     
        
    for(i = 0; i < len / 2; i++ )     
    {   
        temp = *(str + i);     
        *(str + i) = *(str + len - 1 - i);     
        *(str + len - 1 - i) = temp;     
    }     
   
    return str;     
} 
#else
static char *rever_str(char *str)
{
	int len;
	char temp;

	len = strlen(str);

	if(len <= 1)
		return NULL;
	else {
		temp = str[0];
		str[0] = str[len - 1];
		str[len - 1] = '\0';
		rever_str(str+1);
		str[len - 1] = temp;
	}

	return NULL;
}
#endif

int main()
{
	char buf[] = "abcdefg";
	//char *value;

	rever_str(buf);
	printf("%s\n", buf);

	return 0;
}