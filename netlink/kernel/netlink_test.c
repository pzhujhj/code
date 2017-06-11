#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#include <linux/moduleparam.h>
#include <net/netlink.h>

#define NETLINK_TEST 25
static struct sock *nl_sk = NULL;

static int netlink_to_user(char*message, int userpid)
{
	struct nlmsghdr *nlh;
	struct sk_buff *nl_skb;
	int size;
	int msglen;

	msglen = strlen(message);

	size = NLMSG_SPACE(msglen);;
	
	//为新的sk_buff申请空间
	nl_skb = alloc_skb(size, GFP_ATOMIC);
	if (!nl_skb) {
		printk("failed to allocate new skb\n");
		return -1;
	}
	
	//设置netlink消息头部
	nlh = nlmsg_put(nl_skb, 0, 0, 0, msglen, 0);		

	// 设置Netlink的控制块
	NETLINK_CB(nl_skb).portid = userpid;
	NETLINK_CB(nl_skb).dst_group = 0;

	memcpy(NLMSG_DATA(nlh), (char *)message, msglen);
	netlink_unicast(nl_sk, nl_skb, userpid, MSG_DONTWAIT);
}

static void test_nl_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	char *nlmsg;
	int rlen;
	while (skb->len >= NLMSG_SPACE(0)) {
		nlh = (struct nlmsghdr *)skb->data;
		nlmsg = (char *)NLMSG_DATA(nlh);
		
		netlink_to_user("come from kernel", nlh->nlmsg_pid);	
		rlen = NLMSG_ALIGN(nlh->nlmsg_len);
		if (rlen > skb->len)
			rlen = skb->len;

		skb_pull(skb, rlen);
	}	
}

static int __init test_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = test_nl_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
	if (!nl_sk) {
		printk(KERN_ALERT "Error creating socket.\n");
		return -1;
	}

	return 0;	
}

static void __exit test_exit(void)
{
	netlink_kernel_release(nl_sk);
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
