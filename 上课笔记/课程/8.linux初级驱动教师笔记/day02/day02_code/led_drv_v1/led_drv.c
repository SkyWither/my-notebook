//1,包含头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>


#define GPIOZ  0x54004000

unsigned int led_major = 256;
struct class *  led_clz;
struct device * led_dev;

unsigned int * gpioz_mode;
unsigned int * gpioz_odr;

//4,实现设备操作接口函数

int led_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//将gpio设置为输出模式
	*gpioz_mode &=  ~(0x3f<<10);
	*gpioz_mode |= 0x15 << 10;

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
		*gpioz_odr |= 0x7 << 5;
	}else{
		//关灯
		*gpioz_odr &= ~(0x7 << 5);
	}

	return size;
}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	*gpioz_odr &= ~(0x7 << 5);
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
	led_major = register_chrdev(0,"led_drv", &led_fops);
	if(led_major<0){
		printk("register_chrdev error\n");
		return -EINVAL;
	}
#endif 

	//创建类
	led_clz = class_create(THIS_MODULE, "led_class");
	if(!led_clz){
		printk("class_create error\n");
		ret = PTR_ERR(led_clz);
		goto err_unregister_chrdev;
	}

	//2,创建设备结点
	led_dev = device_create(led_clz, NULL, MKDEV(led_major, 5), NULL, "led%d",2);
	if(!led_dev){
		printk("device_create error\n");
		ret = PTR_ERR(led_dev);
		goto err_class_destr;
	}

	//3,硬件初始化
	#if 0
	gpioz_mode =  ioremap(GPIOZ, 4);
	gpioz_odr  =   ioremap(GPIOZ+0x14,4);
   #else
   	gpioz_mode = ioremap(GPIOZ,24);    
	if(!gpioz_mode){
			printk("ioremap error\n");
			ret = PTR_ERR(gpioz_mode);
			goto err_device_destr;
		}
   	gpioz_odr  = gpioz_mode + 5;
   #endif 
	
	
	return 0;
err_device_destr:
  	device_destroy(led_clz, MKDEV(led_major, 5));
err_class_destr:
	class_destroy(led_clz);
err_unregister_chrdev:
	unregister_chrdev(led_major, "led_drv");
	return ret;

}
//3,实现模块卸载(出口)函数
static void __exit led_drv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	iounmap(gpioz_mode);
	device_destroy(led_clz, MKDEV(led_major, 5));
	class_destroy(led_clz);
	unregister_chrdev(led_major, "led_drv");
}

//4,声明与认证
module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
