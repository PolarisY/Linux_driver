/*在不同版本的内核源码上，头文件所在的位置是不同的，比如说在
#include<asm/arch/regs-gpio.h>
#include<mach/regs-gpio.h>
同样是regs-gpio.h,<asm/arch/regs-gpio.h>是在比较低的版本（比如2.6.25）
上位于arch/arm/include/asm中，而mach/regs-gpio.h则是位于
arch/arm/mach-s3c2410/include/mach中，2.6.30版本的内核是这种结构，
所以，要根据所采用的不同的版本内核来修改头文件的位置！*/

#include<linux/module.h>					//最基本的文件，支持动态添加和卸载模块。
#include<linux/init.h>						//初始化头文件
#include<linux/cdev.h>//对字符设备结构cdev以及一系列的操作函数的定义。//包含了cdev 结构及相关函数的定义。
#include<linux/fs.h>//包含了文件操作相关struct的定义，例如大名鼎鼎的struct file_operations
#include<linux/timer.h>						//内核定时器
#include<linux/sched.h>//内核等待队列中要使用的TASK_NORMAL、TASK_INTERRUPTIBLE包含在这个头文件
#include<linux/poll.h>                       //轮询文件
#include<asm/io.h>//I/O头文件，以宏的嵌入汇编程序形式定义对I/O端口操作的函数。
#include<asm/uaccess.h>//包含了copy_to_user、copy_from_user等内核访问用户进程内存地址的函数定义。
#define MINOR_COUNT 1
#define KEY_PERIOD 10
static struct cdev key_cdev;
static dev_t key_no; 
static int key_major,key_minor;
static struct timer_list key_timer;
static struct __wait_queue_head key_wait_queue;
static int *pload=NULL;
static int key_state[4]={1,1,1,1};
static int pressed[4]={0,0,0,0};
static int key_press=0;
//-----------------function declare-----------//
void initial_key_gpio(void);
int key_open(struct inode *pnode, struct file *fp);
int key_release(struct inode *pnode, struct file *fp);
ssize_t key_read(struct file *fp, char __user *buffer, size_t count, loff_t * loff);
unsigned int key_poll(struct file *fp, struct poll_table_struct *poll_table);
//--------------------------------------------//
static struct file_operations key_ops=
{
	.open=key_open,
	.release=key_release,
	.read=key_read,
	.poll=key_poll
};
static void key_timer_handler(unsigned long data)
{
	unsigned int value=0;
	int i=0;
	int state=0;
	value=ioread32(pload+1);
	for(i=0;i<4;i++)
	{
		state=value&(0x1<<i);
		if(state==0)//key down
		{
			pressed[i]++;
		}
		if(pressed[i]==2)//ignore one pulse
		{
			key_state[i]=0;
			key_press=1;//set key down event;
		        wake_up_interruptible((void*)&key_wait_queue);
		}
		if(state>0&&pressed[i])
		{
			pressed[i]=0;
		}
	}
	key_timer.expires=jiffies+KEY_PERIOD;
	add_timer(&key_timer);
	return ;
}
static void key_timer_initial(void)
{
	init_timer(&key_timer);
	key_timer.expires=jiffies+KEY_PERIOD;
	key_timer.function=key_timer_handler;
	add_timer(&key_timer);
}
int key_open(struct inode *pnode, struct file *fp)
{
	initial_key_gpio();
	key_timer_initial();
	init_waitqueue_head((void*)&key_wait_queue);
	return 0;
}
int key_release(struct inode *pnode, struct file *fp)
{
	return 0;
}
ssize_t key_read(struct file *fp, char __user *buffer, size_t count, loff_t * loff)
{
	int retur=0;
	if(!key_press)//key not push down
	{
		if(fp->f_flags&O_NONBLOCK)
		{
			return -EAGAIN;
		}
		else
		{
			wait_event_interruptible(key_wait_queue,key_press);
		}
		
	}
	else//key pushed down
	{
		key_press=0;
		retur=copy_to_user(buffer,key_state,sizeof(key_state));
		memset((void*)key_state,1,sizeof(key_state));
	}
	return 4-retur;

}
unsigned int key_poll(struct file *fp, struct poll_table_struct *poll_table)
{
	unsigned int mask=0;
	poll_wait(fp,&key_wait_queue,poll_table);
	if(key_press)
	{
		mask=mask|POLLIN|POLLRDNORM;
	}
	return mask;
}
/*
  config GPH2_0--- GPH2_3 control register to input mode
*/
void initial_key_gpio(void)
{
	unsigned int value;
	int i=0;
	pload=ioremap(0xE0200C40,32);
	value=ioread32(pload);
	for(i=0;i<15;i++)
	{
		value&=~0x1<<i;
	}
	iowrite32(value,pload);
}
static void alloc_dev_number(void)
{
	alloc_chrdev_region(&key_no,0,MINOR_COUNT,"key driver");
	key_major=MAJOR(key_no);
	key_minor=MINOR(key_no);
	printk("key_major= %d,key_minor=%d\n",key_major,key_minor);
}
static void key_cdev_init(void)
{
	cdev_init(&key_cdev,&key_ops);
	key_cdev.owner=THIS_MODULE;
	key_cdev.dev=key_no;
	key_cdev.count=MINOR_COUNT;
	key_cdev.ops=&key_ops;
	cdev_add(&key_cdev,key_no,MINOR_COUNT);
}
static int __init key_init_func(void)
{
	alloc_dev_number();
	key_cdev_init();
	return 0;
}
static void __exit key_exit_func(void)
{
	cdev_del(&key_cdev);
	unregister_chrdev_region(key_no,MINOR_COUNT);	
	del_timer(&key_timer);
	iounmap(pload);
}
module_init(key_init_func);
module_exit(key_exit_func);
