#include<linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/string.h>
#include<linux/inet.h>
#include <linux/ip.h>

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"

static int __init hello_init(void)
{
        __be32 result;
		unsigned long sip;
        char ip[] = "192.168.10.188";

        printk("==1===%x\n", in_aton("192.168.10.188"));
		sip = in_aton("192.168.10.188");
		printk("==sip=%x\n", sip);

		printk(""NIPQUAD_FMT" \n", NIPQUAD(sip));

        in4_pton(ip, strlen(ip), (u8 *)&result, '\0', NULL);
        printk("===2==%x\n", result);
		printk(""NIPQUAD_FMT" \n", NIPQUAD(result));

        return 0;
}

static void __exit hello_exit(void)
{
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");

