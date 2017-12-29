#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>
#include<asm/io.h>
static struct cdev show_cdev;//define cdev variable
static dev_t dev_no;//define device number
static  int *pload=NULL;
static int show_open(struct inode * pnod,struct file *fp);
static int show_release (struct inode * pnod, struct file *fp);
ssize_t show_read (struct file *fp, char __user * buffer, size_t count , loff_t *offset);
ssize_t show_write (struct file *fp, const char __user *buffer, size_t count, loff_t *offset);
/*执行设备IO 控制命令,不使用BLK 的文件系统*/
long show_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long count);
void initial_gpd0_0(void);
static struct file_operations show_fops=
{
	.open=show_open,
	.release=show_release,
	.read=show_read,
	.write=show_write,
	.unlocked_ioctl=show_unlocked_ioctl
};
static int major=0,minor=0;
static int k_strlen(const char *str)
{
	int count=0;
	while(*str!='\0')
	{
		count++;
		str++;
	}
	return count;
}
//int (*open) (struct inode *, struct file *);
static int show_open(struct inode * pnod,struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	initial_gpd0_0();
	return 0;
}
	//int (*release) (struct inode *, struct file *);
static int show_release (struct inode * pnod, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
//	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t show_read (struct file *fp, char __user * buffer, size_t count , loff_t *offset)
{
	char string[]="HELLO,EVERYONE\n";
	int num=0;
	/*printk("%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
	内核调试常会打印 文件名 函数名 行号*/
	printk("%s\n",__FUNCTION__);/*__FUNCTION__ 是宏定义，它的作用就是输出其所在函数的函数名*/
	num=copy_to_user(buffer,string,k_strlen(string)+1);
	return num;
}
//	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
ssize_t show_write (struct file *fp, const char __user *buffer, size_t count, loff_t *offset)
{
	char string[200];
	int num=0;
	printk("%s\n",__FUNCTION__);
	num=copy_from_user(string,buffer,count);
	printk("KERNEL:   %s\n",string);
	return 0;
}
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
void beep_start(void)
{
	unsigned int value=0;
	value=ioread32(pload+1);//data register
	value|=0x1<<0;
	iowrite32(value,pload+1);
}
void beep_stop(void)
{
	unsigned int value=0;
	value=ioread32(pload+1);//data register
	value&=~0x1<<0;
	iowrite32(value,pload+1);
}
//不使用BLK的文件系统，将使用此种函数指针代替ioctl
long show_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long count)
{
	printk("%s cmd= %d\n",__FUNCTION__,cmd);
	switch(cmd)
	{
		case 0://beep stop
			beep_stop();
		break;
		case 1://beep start
			beep_start();
		break;
	}
	return count;
}
void initial_gpd0_0(void)
{
	unsigned int value=0;
	pload=ioremap(0XE02000A0,16);//configure register
	printk("pload= 0x%x\n",(unsigned int)pload);
	value=ioread32(pload);
	value&=~0x1<<3;
	value&=~0x1<<2;
	value&=~0x1<<1;
	value|=0x1<<0;
	iowrite32(value,pload);
}
static int __init hello_init(void)
{
	if(major>0)
	{
		dev_no=MKDEV(major,minor);
		register_chrdev_region(dev_no,1,"hello");
	}
	else
	{
		alloc_chrdev_region(&dev_no,0,1,"hello");
		major=MAJOR(dev_no);
		minor=MINOR(dev_no);
	}
	printk("major= %d,minor= %d\n",major,minor);
	printk("%s\n",__FUNCTION__);
	cdev_init(&show_cdev,&show_fops);
	show_cdev.owner=THIS_MODULE;
	show_cdev.ops=&show_fops;
	show_cdev.dev=dev_no;
	show_cdev.count=1;
	cdev_add(&show_cdev,dev_no,1);
	return 0;
}
static void __exit hello_exit(void)
{
	printk("%s\n",__FUNCTION__);
	unregister_chrdev_region(dev_no,1);
	cdev_del(&show_cdev);
	iounmap(pload);
	return;
}
module_init(hello_init);
module_exit(hello_exit);
