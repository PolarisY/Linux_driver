#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
static struct cdev show_cdev;//define cdev variable
static dev_t dev_no;//define device number
static int show_open(struct inode * pnod,struct file *fp);
static int show_release (struct inode * pnod, struct file *fp);
static struct file_operations show_fops=
{
	.open=show_open,
	.release=show_release,
};
static int major=0,minor=0;
	//int (*open) (struct inode *, struct file *);
static int show_open(struct inode * pnod,struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
	//int (*release) (struct inode *, struct file *);
static int show_release (struct inode * pnod, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
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
	return;
}
module_init(hello_init);
module_exit(hello_exit);
