#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kernel.h>  
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#ifndef CHAR_MAJOR
#define CHAR_MAJOR 0   /* dynamic major by default */
#endif

static int char_major = CHAR_MAJOR;
static int char_minor = 0;

#define CHRMEM_SIZE 0x1000
#define MEM_CLEAR   0x1

struct chr_dev
{
	struct cdev cdev;
	unsigned char mem[CHRMEM_SIZE];
};

static struct chr_dev* char_devp = NULL;
static struct class *char_dev_class = NULL;


int chr_open(struct inode* inode, struct file* filp)
{
	filp->private_data = char_devp;
	return 0;
}

int chr_release(struct inode* inode, struct file* filp)
{
	return  0;
}

static long chr_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
	struct chr_dev* dev = filp->private_data;

	printk("chr_ioctl  %s\n", dev->mem + arg);
	
	switch(cmd)
	{
		case MEM_CLEAR:
			memset(dev->mem, 0, CHRMEM_SIZE);
			break;
			
		default:
			return -EINVAL;
	}
	
	return 0;
}

static ssize_t chr_read(struct file* filp, char __user* buf, size_t size, loff_t* ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct chr_dev* dev = filp->private_data;
	
	if(p >= CHRMEM_SIZE)
	{
		return 0;
	}
	
	if(count > CHRMEM_SIZE - p)
	{
		return 0;
	}
	
	if(copy_to_user(buf, (void*)(dev->mem + p), count))
	{
		return -EINVAL;
	}
	else
	{
		*ppos += count;
		ret = count;
	}

	printk("copy_to_user mesage:%s\n", buf);
	
	return ret;
}

static ssize_t chr_write(struct file* filp, const char __user* buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct chr_dev* dev = filp->private_data;
	
	if(p >= CHRMEM_SIZE)
	{
		return 0;
	}
	
	if(count > CHRMEM_SIZE - p)
	{
		count = CHRMEM_SIZE - p;
	}
	
	if(copy_from_user(dev->mem + p, buf, count))
	{
		ret = -EINVAL;
	}
	else
	{
		*ppos += count;
		ret = count;
	}

	printk("copy_from_user message:%s\n", dev->mem+p);
	
	return ret;
}

static loff_t chr_llseek(struct file* filp, loff_t offset, int orig)
{
	loff_t ret = 0;

	printk("chr_llseek \n");
	
	/* orig can be SEEK_SET, SEEK_CUR, SEEK_END */
	switch(orig)
	{
		case 0:
			if(offset < 0)
			{
				ret = -EINVAL;
				break;
			}
			
			if((unsigned int) offset > CHRMEM_SIZE)
			{
				ret = -EINVAL;
				break;
			}
			
			filp->f_pos = (unsigned int) offset;
			ret = filp->f_pos;
			break;
			
		case 1:
			if((filp->f_pos + offset) > CHRMEM_SIZE)
			{
				ret = -EINVAL;
				break;
			}
			
			if((filp->f_pos + offset) < 0)
			{
				ret = -EINVAL;
				break;
			}
			
			filp->f_pos += offset;
			ret = filp->f_pos;
			break;
			
		default:
			ret = - EINVAL;
			break;
	}
	
	return ret;
}

static const struct file_operations chr_ops = 
{
	.owner    = THIS_MODULE,
	.llseek   = chr_llseek,
	.read     = chr_read,
	.write    = chr_write,
	.unlocked_ioctl  = chr_ioctl,   //新版本中ioctl已经去掉，主要用unlocked_ioctl和compat_ioctl(兼容老版本)
	.open     = chr_open,
	.release  = chr_release
};

static void chr_setup_cdev(struct chr_dev* dev, int index)
{
	int err;
	int devno = MKDEV(char_major, index);
	
	cdev_init(&dev->cdev, &chr_ops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &chr_ops;
	
	err = cdev_add(&dev->cdev, devno, 1);
	if(err)
		printk("Error %d add cdevdemo %d\n",err, index);
}

static int __init  chr_init(void)
{
	int result;
	dev_t ndev;

	/*获取主设备号，静态和动态方式*/
	if (char_major) {
		ndev = MKDEV(char_major, char_minor);
		result = register_chrdev_region(ndev, 1, "chr_dev");
	}else {
		result = alloc_chrdev_region(&ndev, 0, 1, "chr_dev");
		char_major = MAJOR(ndev);
	}

    if(result < 0 )  
    { 
    	printk(KERN_WARNING "char: can't get major %d\n", char_major);
        return result;  
    } 	
	
	printk("chr_init(): major = %d, minor = %d\n", MAJOR(ndev), MINOR(ndev));
	
	char_devp = kmalloc(sizeof(struct chr_dev), GFP_KERNEL);
	if(!char_devp)
	{
		result = -ENOMEM;
		goto fail;
	}
	
	memset(char_devp, 0, sizeof(struct chr_dev));
	chr_setup_cdev(char_devp, char_minor);

	/*在驱动模块初始化函数中实现设备节点自动创建*/
	char_dev_class = class_create(THIS_MODULE, "char_class");
	if (IS_ERR(char_dev_class))  
	{  
		printk("Err: failed in creating char_dev class.\n");  
		goto err;  
	}
	device_create(char_dev_class, NULL, ndev, NULL, "char_device"); 

	
	return 0;
err:
	kfree(char_devp);
	
fail:
	unregister_chrdev_region(ndev, 1);
	return result;
}

static void  __exit chr_exit(void)
{
	device_destroy(char_dev_class, MKDEV(char_major, char_minor));
	class_destroy(char_dev_class);
	cdev_del(&char_devp->cdev);
	kfree(char_devp);
	unregister_chrdev_region(MKDEV(char_major, char_minor), 1);
}

module_init(chr_init);
module_exit(chr_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xxx.com");
MODULE_DESCRIPTION("A simple device example!");

