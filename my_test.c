#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/netlink.h>

#define NETLINK_TEST 22

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Koorey King");

struct sock *nl_sk = NULL;
#if 0
void nl_data_ready (struct sock *sk, int len)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh = NULL;

    while((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL)
    {
          nlh = (struct nlmsghdr *)skb->data;
          printk("%s: received netlink message payload: %s \n", __FUNCTION__, (char*)NLMSG_DATA(nlh));
          kfree_skb(skb);
    }
    printk("recvied finished!\n");
}
#endif
void nl_data_ready (struct sk_buff *__skb)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	u32 pid;
	int rc;
	int len = NLMSG_SPACE(1200);
	char str[100];

	printk("net_link: data is ready to read.\n");
	skb = skb_get(__skb);

	if (skb->len >= NLMSG_SPACE(0)) {
		nlh = nlmsg_hdr(skb);
		printk("net_link: recv %s.\n", (char *)NLMSG_DATA(nlh));
		memcpy(str,NLMSG_DATA(nlh), sizeof(str)); 
		pid = nlh->nlmsg_pid; /*pid of sending process */
		printk("net_link: pid is %d\n", pid);
		kfree_skb(skb);
	}
}
static int __init myinit_module(void)
{
	printk("my ntelink in \n");
	struct netlink_kernel_cfg cfg = {
		.input = nl_data_ready,
	};
   // nl_sk = netlink_kernel_create(NETLINK_TEST,0,nl_data_ready,THIS_MODULE);
	nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);

	if (!nl_sk) {
		printk(KERN_ERR "net_link: Cannot create netlink socket.\n");
		return -EIO;
	}

	printk("net_link: create socket ok.\n");
    return 0;
}

static void __exit mycleanup_module(void)
{
    printk("my netlink out!\n");
    sock_release(nl_sk->sk_socket);
}

module_init(myinit_module);
module_exit(mycleanup_module);
