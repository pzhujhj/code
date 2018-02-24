#include <stdio.h>

/*判断素数的方法，最笨拙的方法不用说了，第二种采用蒙哥马利快速幂算法实现，
*第二种方法没有理解算法思想，记住有这个方法，效率也很好,参考地址：
http://blog.csdn.net/qq_32742637/article/details/51706552*/

#if 0
static int prime_func1(int num)
{
	int i;
	
	for(i = 2; i <= num-1; i++) {
		if((num % i) == 0) {
			return 0;
		}
	}

	return 1;
}
#else
unsigned int Montgmery(unsigned int num, unsigned index, unsigned int mod)//蒙哥马利快速幂算法  (num^index)%mod  
{  
    unsigned int tmp = 1;//保存奇数是剩下的那个数  
    num %= mod;  // 假设(2^7)%3     2%3  
    while (index > 1)  
    {  
        if (index & 1)  //按位与 ，除最低位不变其他位置0 ，如果为1 说明是奇数，否则偶数  
        {  
            tmp = (tmp*num) % mod;// 假设(2^7)%3  第一次为 1%3=1  (1%3)(2%3)%3=2%3    第二次 为((2%3)*(4%3))%3=8%3  
        }  
        num = (num*num) % mod;//(2%3)(2%3)%3=4%3    (4%3)(4%3)%3=16%3  
        index /= 2;  
    }  
    unsigned result = (num*tmp) % mod;     //(16%3)(8%3)%3=(2^7)%3  
    return result;  
} 

static int prime_func2(unsigned int num)
{
	int i = 0;
	
	if(num < 2)
		return -1;

    static unsigned int PrimeList[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31 ,37, 41,
    									43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};//质数表  
    const int count = sizeof(PrimeList) / sizeof(unsigned int);//求出置数表中数的个数  
    //printf("%d\n", count);  
      
    for (i = 0; i < count; i++)//循环质数表中的每一个数  
    {  
        if (num == PrimeList[i])//如果输入的数在质数表中则返回  这句绝对不能删，删了就不对了  
        {  
            return 1;  
        }  
        //printf("Montgmery=%d\n", Montgmery(PrimeList[i], num - 1, num));  
        if (Montgmery(PrimeList[i], num - 1, num) != 1)//按照表中的质素算，如果全都为1则可以大概说num是质数  
        {  
            return -1;  
        }  
    }
	
    return 1; 
}
#endif
int main()
{
	int number, ret;
	printf("please input number:\n");

	while(scanf("%d", &number)) {
		//ret = prime_func1(number);
		ret = prime_func2(number);
		if(ret == 1)
			printf("this is prime\n");
		else
			printf("this is not prime\n");
		
		printf("please input number:\n");
	}
	return 0;
}