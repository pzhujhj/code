#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/version.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <linux/netdevice.h>

#define ETH "ens33"//用的本机无线网卡设备
#define SIP "192.168.1.27"//随意
#define DIP "192.168.1.55"//
#define SPORT 39804
#define DPORT 6980

#define TCP_STRUCT_SIZE sizeof(struct tcphdr)
#define UDP_STRUCT_SIZE sizeof(struct udphdr)
#define IP_STRUCT_SIZE sizeof(struct iphdr)
#define ETH_STRUCT_SIZE sizeof(struct ethhdr)

unsigned char SMAC[ETH_ALEN] =
{0x0, 0x0c, 0x29, 0xc1, 0xe6, 0x62};//本机无线网卡硬件地址
unsigned char DMAC[ETH_ALEN] = 
{0x70, 0x8b, 0xcd, 0x80, 0x13, 0x96};//本机网卡硬件地址


#define NIPQUAD(addr) \
  ((unsigned char *)&addr)[0], \
  ((unsigned char *)&addr)[1], \
  ((unsigned char *)&addr)[2], \
  ((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"


static char *inet_ntoa_r(struct in_addr ina, char *buf)
{
	unsigned char *ucp = (unsigned char *)&ina;

	sprintf(buf, "%d.%d.%d.%d",
		ucp[0] & 0xff,
		ucp[1] & 0xff,
		ucp[2] & 0xff,
		ucp[3] & 0xff);
		
	return buf;
}

static int construct_tcp_packets(char *eth, u_char *smac, u_char *dmac,
		u_char *pkt, int pkt_len, u_long sip, u_long dip, u_short sport, u_short dport)
{
	struct sk_buff *skb = NULL;
	struct net_device *dev = NULL;
	struct tcphdr *tcph = NULL;
	struct iphdr *iph = NULL;
	struct ethhdr *ethdr = NULL;
	u_char *pdata = NULL;
	int nret = 1;
	printk("---tcp test\n");

	do { 
		if(NULL == smac || NULL == dmac)
			break;

		if(NULL == (dev = dev_get_by_name(&init_net, eth)))
			break;
		
		skb = alloc_skb(pkt_len + TCP_STRUCT_SIZE + IP_STRUCT_SIZE + ETH_STRUCT_SIZE, GFP_ATOMIC);
		if (NULL == skb)
			break;

		skb_reserve(skb, pkt_len + sizeof(struct tcphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr));


		skb->dev = dev;
		skb->pkt_type = PACKET_OTHERHOST;
		skb-> protocol = __constant_htons(ETH_P_IP);
		skb->ip_summed = CHECKSUM_NONE;
		skb->priority = 0;


		pdata = skb_push(skb, pkt_len);
		tcph = (struct tcphdr*)skb_push(skb, TCP_STRUCT_SIZE);
		iph = (struct iphdr*)skb_push(skb, IP_STRUCT_SIZE);
		ethdr = (struct ethhdr*)skb_push(skb, ETH_STRUCT_SIZE);

		memcpy(pdata, pkt, pkt_len);


		tcph->check = 0; // No TCP checksum yet.
		tcph->source = sport; // Source TCP Port.
		tcph->dest = dport; // Destination TCP Port.
		tcph->seq = htonl(1234 - 1); // Current SEQ minus one is used for TCP keepalives.
		tcph->ack_seq = htonl( 2345 - 1); // Ummm not sure yet.
		tcph->res1 = 0; // Not sure.
		tcph->doff = 5; // TCP Offset.	At least 5 if there are no TCP options.
		tcph->fin = 0; // FIN flag.
		tcph->syn = 0; // SYN flag.
		tcph->rst = 0; // RST flag.
		tcph->psh = 0; // PSH flag.
		tcph->ack = 1; // ACK flag.
		tcph->urg = 0; // URG flag.
		tcph->ece = 0; // ECE flag? It should be 0.
		tcph->cwr = 0; // CWR flag? It should be 0.


		iph->version = 4;
		iph->ihl = IP_STRUCT_SIZE >> 2;//ip头长度
		iph->frag_off = 0;
		iph->protocol = IPPROTO_TCP;
		iph->tos = 0;
		iph->daddr = dip;
		iph->saddr = sip;
		iph->ttl = 0x40;
		iph->tot_len = __constant_htons(skb->len);
		iph->check = 0;
		iph->check = ip_fast_csum((unsigned char*)iph, iph->ihl);//计算校验和
		skb->csum = skb_checksum(skb, iph->ihl*4, skb->len - iph->ihl*4, 0);//skb校验和计算
		tcph->check = csum_tcpudp_magic(sip, dip, skb->len - iph->ihl*4, IPPROTO_TCP, skb->csum);//udp和tcp伪首部校验和

	    //链路层数据填充
	    memcpy(ethdr->h_dest, dmac, ETH_ALEN);
	    memcpy(ethdr->h_source, smac, ETH_ALEN);
	    ethdr->h_proto = __constant_htons(ETH_P_IP);

		//调用内核协议栈函数，发送数据包
		if(dev_queue_xmit(skb) < 0)
		{
			printk("dev_queue_xmit error\n");
			break;
		}
		nret = 0;//这里是必须的
		printk("tcp dev_queue_xmit correct\n");
	}while(0);

	if(0 != nret && NULL != skb)
	{
		dev_put(dev);//减少设备的引用计数
		kfree_skb(skb);//销毁数据包
	}


	return nret;//F_ACCEPT;

}

static int construct_udp_packets(char *eth, u_char *smac, u_char *dmac,
		u_char *pkt, int pkt_len, u_long sip, u_long dip, u_short sport, u_short dport)
{
	struct sk_buff *skb = NULL;
	struct net_device *dev = NULL;
	struct udphdr *udph = NULL;
	struct iphdr *iph = NULL;
	struct ethhdr *ethdr = NULL;
	u_char *pdata = NULL;
	int nret = 1;

    if(NULL == smac || NULL == dmac)
        goto out;

	/*linux可以使用dev_get_by_name函数取得设备指针，但是使用是需要注意，
	使用过dev_get_by_name函数后一定要使用dev_put(pDev)函数取消设备引用，
	不然可能导致GET的设备无法正常卸载*/
	if(NULL == (dev = dev_get_by_name(&init_net, eth)))
		goto out;

	//create skb 
	skb = alloc_skb(pkt_len + UDP_STRUCT_SIZE + IP_STRUCT_SIZE + ETH_STRUCT_SIZE, GFP_ATOMIC);
	if (NULL == skb)
		goto out;
	
	/*为skb预留空间，方便后面skb_buff协议封装,skb_reserve()只能用于空的SKB，
	通常会在分配SKB之后就调用该函数，此时data和tail指针还一同指向数据区的起始位置*/
	skb_reserve(skb, pkt_len + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ethhdr));

	//skb 字节填充
	skb->dev = dev;
	/*	PACKET_HOST 这是一个发往本机的数据包。
		PACKET_BROADCAST 广播数据包。
		PACKET_MULTICAST 多播数据包。
		PACKET_OTHERHOST 该数据包是发往其它机器的，如果本机没有被配置为转发功能，该数据包即被丢弃*/
	skb->pkt_type = PACKET_OTHERHOST;
	skb-> protocol = __constant_htons(ETH_P_IP); //这句是什么意思?
	skb->ip_summed = CHECKSUM_NONE;
	skb->priority = 0;

	/*数据包封装 分别压入应用层，传输层，网络层，链路层栈帧*/
	pdata = skb_push(skb, pkt_len);
	udph = (struct udphdr*)skb_push(skb, UDP_STRUCT_SIZE);
	iph = (struct iphdr*)skb_push(skb, IP_STRUCT_SIZE);
	ethdr = (struct ethhdr*)skb_push(skb, ETH_STRUCT_SIZE);

	//应用层数据填充
	memcpy(pdata, pkt, pkt_len);

	//传输层udp数据填充
	memset(udph, 0, UDP_STRUCT_SIZE);
	udph->source = sport;
	udph->dest = dport;
	udph->len = htons(UDP_STRUCT_SIZE + pkt_len);//主机字节序转网络字节序
	udph->check = 0;//skb_checksum之前必须置0.协议规定

	//网络层数据填充
	iph->version = 4;
	iph->ihl = IP_STRUCT_SIZE >> 2;//ip头长度
	iph->frag_off = 0;
	iph->protocol = IPPROTO_UDP;
	iph->tos = 0;
	iph->daddr = dip;
	iph->saddr = sip;
	iph->ttl = 0x40;
	iph->tot_len = __constant_htons(skb->len);
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char*)iph, iph->ihl);//计算校验和
	skb->csum = skb_checksum(skb, iph->ihl*4, skb->len - iph->ihl*4, 0);//skb校验和计算
	udph->check = csum_tcpudp_magic(sip, dip, skb->len - iph->ihl*4, IPPROTO_UDP, skb->csum);//udp和tcp伪首部校验和

    //链路层数据填充
    memcpy(ethdr->h_dest, dmac, ETH_ALEN);
    memcpy(ethdr->h_source, smac, ETH_ALEN);
    ethdr->h_proto = __constant_htons(ETH_P_IP);

	//调用内核协议栈函数，发送数据包
	if(dev_queue_xmit(skb) < 0)
	{
		printk("dev_queue_xmit error\n");
		goto out;
	}
	nret = 0;//这里是必须的
	printk("dev_queue_xmit correct\n");
	//出错处理
out:
	/*下面的0!=nret是必须的，前面即使不执行goto out，下面的语句程序也会执行，
	如果不加0!=nret语句，那么前面dev_queue_xmit返回之后（已经kfree_skb一次了），
	再进入下面的语句第二次执行kfree_skb，就会导致系统死机*/
	//关键在于知道dev_queue_xmit内部调用成功后，会kfree_skb，以及goto语句的作用
	
	if(0 != nret && NULL != skb)//这里前面的nret判断是必须的，不然必定死机
	{
		dev_put(dev);//减少设备的引用计数
		kfree_skb(skb);//销毁数据包
	}
	
	return nret;//F_ACCEPT;
}

static int  print_host(unsigned char *payload, unsigned int len)
{
	char *msg_tmp = NULL;
	int ret = 0;
	
	if (!strncmp(payload, "GET", 3) || !strncmp(payload, "POST", 4)) {
		//printk("payload:%s\n", payload);
		//printk("len=%d\n", len);
		msg_tmp = strstr(payload, "Host:");
		//printk("msg_tmp:%s\n", msg_tmp);
		ret = -1;
	}

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
static unsigned int hook_func(void *priv,
	struct sk_buff *skb,
	const struct nf_hook_state *state)
#else
static unsigned int hook_func(unsigned int hooknum,  
	struct sk_buff *skb,  
	const struct net_device *in,  
	const struct net_device *out,  
	int (*okfn)(struct sk_buff *))
#endif
{
	__be32 sip, dip;
	char sipstr[32] = {0}, dipstr[32] = {0};
	struct in_addr saddr, daddr;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;

	unsigned char *payload = NULL;
	unsigned short sport = 0;  
	unsigned short dport = 0;
	unsigned int len_total = 0, len_tcp = 0;

	unsigned char *pdata = "hello kernel";
	int ret = NF_ACCEPT; 

	struct iphdr *iph = ip_hdr(skb);

	sip = iph->saddr;
	dip = iph->daddr;

	saddr.s_addr = iph->saddr;
	daddr.s_addr = iph->daddr;

	memset(sipstr, 0, sizeof(sipstr));
	memset(dipstr, 0, sizeof(dipstr));

	len_total= ntohs(iph->tot_len); 
	
	if (iph->protocol == IPPROTO_TCP) {
	//	printk("protocol tcp\n");
	#if 0
		tcph = (struct tcphdr *)((char *)skb->data + (int)(iph->ihl * 4));
	#else
		tcph = tcp_hdr(skb);
	#endif
		sport = ntohs(tcph->source);
    	dport = ntohs(tcph->dest);
		//printk("sport=%d dport=%d\n", sport, dport);

		inet_ntoa_r(saddr, sipstr);
		//printk("tcp sipstr=%s\n", sipstr);

		inet_ntoa_r(daddr, dipstr);
		//printk("tcp dipstr=%s\n", dipstr);

		len_tcp = len_total - iph->ihl*4 - tcph->doff*4;
		payload = (unsigned char *)((unsigned char*)tcph + (tcph->doff * 4));
		if (print_host(payload, len_tcp) <0)
			ret = NF_DROP;

	}else if (iph->protocol == IPPROTO_UDP) {
//		printk("protocol udp\n");
//		udph = (struct udphdr *)((char *)skb->data + (int)(iph->ihl * 4));	
//		sport=ntohs(udph->source);	
//		dport=ntohs(udph->dest); 
	}
	else if (iph->protocol == IPPROTO_ICMP) {
		printk("protocol icmp\n");
		printk("sip:"NIPQUAD_FMT", dip:"NIPQUAD_FMT"\n", NIPQUAD(sip), NIPQUAD(dip));
		//sprintf(ipstr, ""NIPQUAD_FMT"", NIPQUAD(dip));
		//printk("ipstr=%s\n", ipstr);

		inet_ntoa_r(saddr, sipstr);
		printk("sipstr=%s\n", sipstr);

		inet_ntoa_r(daddr, dipstr);
		printk("dipstr=%s\n", dipstr);
		//ret = construct_udp_packets(ETH, SMAC, DMAC, pdata, strlen(pdata), in_aton(SIP), in_aton(DIP), htons(SPORT), htons(DPORT));
		ret = construct_tcp_packets(ETH, SMAC, DMAC, pdata, strlen(pdata), in_aton(SIP), in_aton(DIP), htons(SPORT), htons(DPORT));
	}


	//printk("sip:"NIPQUAD_FMT", dip:"NIPQUAD_FMT"\n", NIPQUAD(sip), NIPQUAD(dip));

    return ret;  
}

/*************************************
根据内核版本不一样定义也有差别
struct nf_hook_ops
{
    struct list_head list;    //链表成员
   	// User fills in from here down.
    nf_hookfn *hook;     //钩子函数指针
    struct module *owner;
    int pf;              //协议簇，对于ipv4而言，是PF_INET
    int hooknum;      //hook类型
    //Hooks are ordered in ascending priority.
    int priority;    //优先级
};
***************************************/

/* A netfilter instance to use */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
static struct nf_hook_ops nfho = {
	.hook = hook_func,  
    .pf = PF_INET,  
    .hooknum = NF_INET_LOCAL_OUT,
    .priority = NF_IP_PRI_FIRST,
};
#else
static struct nf_hook_ops nfho = {
	.hook = hook_func,  
    .pf = PF_INET,  
    .hooknum = NF_INET_PRE_ROUTING,  
    .priority = NF_IP_PRI_FIRST,  
	.owner = THIS_MODULE,
};
#endif

static int nf_init(void)  
{
	if (nf_register_hook(&nfho))
		printk("nf_register_hook error!\n");
	
    return 0;  
}
static void  nf_cleanup(void)  
{  
    nf_unregister_hook(&nfho);  
}  
  
  
module_init(nf_init);  
module_exit(nf_cleanup);  
MODULE_AUTHOR("xxxx");  
MODULE_LICENSE("GPL");  

