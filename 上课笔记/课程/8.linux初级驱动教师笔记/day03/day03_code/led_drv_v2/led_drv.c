//1,包含头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>


#define GPIOZ  0x54004000

//全局设备对象类型
struct mp157_led{
	unsigned int major;
	struct class *  clz;
	struct device * dev;

	unsigned int * mode;
	unsigned int * odr;
};

struct mp157_led  *led_dev;



//4,实现设备操作接口函数

int led_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//将gpio设置为输出模式
	*led_dev->mode &=  ~(0x3f<<10);
	*led_dev->mode  |= 0x15 << 10;

	return 0;
}
ssize_t led_drv_write(struct file *filp, const char __user *buf, size_t size, loff_t *flags)
{
	int ret;
	int value;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//将应用数据转为内核数据
	ret = copy_from_user(&value, buf, size);
	if(ret > 0){
		printk("copy_from_user error\n");
		return -EINVAL;
	}

	//判断应用传递的数据 1---开灯，0 --- 关灯
	if(value){
		//开灯
		*led_dev->odr |= 0x7 << 5;
	}else{
		//关灯
		*led_dev->odr &= ~(0x7 << 5);
	}

	return size;
}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	*led_dev->odr &= ~(0x7 << 5);
	return 0;
}

const struct file_operations led_fops = {
	.open	=	led_drv_open,
	.write	=	led_drv_write,
	.release = led_drv_close,
};

//2,实现模块加载(入口)函数
static int __init led_drv_init(void)
{
	int ret;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//0，申请全局设备对象空间
	led_dev = kzalloc(sizeof(*led_dev), GFP_KERNEL);
	if(!led_dev){
		printk("kzalloc error");
		return -ENOMEM;
	}
	//1,申请设备号
#if 0
	//静态指定
	ret = register_chrdev(led_major,"led_drv", &led_fops);
	if(ret<0){
		printk("register_chrdev error\n");
		return -EINVAL;
	}
#else
	//动态分配
	led_dev->major = register_chrdev(0,"led_drv", &led_fops);
	if(led_dev->major<0){
		printk("register_chrdev error\n");
		ret = led_dev->major;
	    goto  err_kfree;
	}
#endif 

	//创建类
	led_dev->clz = class_create(THIS_MODULE, "led_class");
	if(!led_dev->clz ){
		printk("class_create error\n");
		ret = PTR_ERR(led_dev->clz );
		goto err_unregister_chrdev;
	}

	//2,创建设备结点
	led_dev->dev = device_create(led_dev->clz , NULL, MKDEV(led_dev->major, 5), NULL, "led%d",2);
	if(!led_dev->dev){
		printk("device_create error\n");
		ret = PTR_ERR(led_dev->dev);
		goto err_class_destr;
	}

	//3,硬件初始化
	#if 0
	gpioz_mode =  ioremap(GPIOZ, 4);
	gpioz_odr  =   ioremap(GPIOZ+0x14,4);
   #else
   	led_dev->mode = ioremap(GPIOZ,24);    
	if(!led_dev->mode){
			printk("ioremap error\n");
			ret = PTR_ERR(led_dev->mode);
			goto err_device_destr;
		}
   	led_dev->odr  = led_dev->mode + 5;
   #endif 
	
	
	return 0;
err_device_destr:
  	device_destroy(led_dev->clz, MKDEV(led_dev->major, 5));
err_class_destr:
	class_destroy(led_dev->clz);
err_unregister_chrdev:
	unregister_chrdev(led_dev->major, "led_drv");
err_kfree:
	kfree(led_dev);
	return ret;

}
//3,实现模块卸载(出口)函数
static void __exit led_drv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	iounmap(led_dev->mode);
	device_destroy(led_dev->clz, MKDEV(led_dev->major, 5));
	class_destroy(led_dev->clz);
	unregister_chrdev(led_dev->major, "led_drv");
	kfree(led_dev);
}

//4,声明与认证
module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
