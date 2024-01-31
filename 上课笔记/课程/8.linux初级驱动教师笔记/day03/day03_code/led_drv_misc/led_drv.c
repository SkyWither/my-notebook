//1,包含头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>

#define GPIOZ  0x54004000

//全局设备对象类型
struct mp157_led{
	struct miscdevice  misc;

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
	//1，申请全局设备对象空间
	led_dev = kzalloc(sizeof(*led_dev), GFP_KERNEL);
	if(!led_dev){
		printk("kzalloc error");
		return -ENOMEM;
	}
	//2,初始化杂项设备对象
	led_dev->misc.fops  =  &led_fops;  //设备操作对象地址
	led_dev->misc.minor = 7;          //次设备号
	led_dev->misc.name =  "led01";    //设备结点名称

	//3,注册杂项设备对象
	ret = misc_register(&led_dev->misc);
	if(ret < 0){
		printk("misc_register error\n");
		goto err_kfree;
	}


	//4,硬件初始化
	#if 0
	gpioz_mode =  ioremap(GPIOZ, 4);
	gpioz_odr  =   ioremap(GPIOZ+0x14,4);
   #else
   	led_dev->mode = ioremap(GPIOZ,24);    
	if(!led_dev->mode){
			printk("ioremap error\n");
			ret = PTR_ERR(led_dev->mode);
			goto err_misc_deregister;
		}
   	led_dev->odr  = led_dev->mode + 5;
   #endif 
	
	
	return 0;
err_misc_deregister:
	misc_deregister(&led_dev->misc);
err_kfree:
	kfree(led_dev);
	return ret;

}
//3,实现模块卸载(出口)函数
static void __exit led_drv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	iounmap(led_dev->mode);
	misc_deregister(&led_dev->misc);
	kfree(led_dev);
}

//4,声明与认证
module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
