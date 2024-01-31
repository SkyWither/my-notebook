//1,包含头文件
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>

#include "stm32mp157_gpio.h"
#include "led_plat.h"



//全局设备对象类型
struct mp157_led{
	struct miscdevice  misc;
	gpio_t *gpioz;
	struct device_node *np;
	int mode_clear;
	int mode_data;
	int odr;
	int shift;
};

struct mp157_led  *led_dev;



//4,实现设备操作接口函数
int led_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//将gpio设置为输出模式
	led_dev->gpioz->MODER &=  ~(led_dev->mode_clear << led_dev->shift *2);
	led_dev->gpioz->MODER |= led_dev->mode_data << led_dev->shift *2;

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
		led_dev->gpioz->ODR |= led_dev->odr << led_dev->shift;
	}else{
		//关灯
		led_dev->gpioz->ODR &= ~(led_dev->odr  << led_dev->shift);
	}

	return size;
}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	led_dev->gpioz->ODR &= ~(led_dev->odr  << led_dev->shift);
	return 0;
}

const struct file_operations led_fops = {
	.open	=	led_drv_open,
	.write	=	led_drv_write,
	.release = led_drv_close,
};

//实现probe
int led_pdrv_probe(struct platform_device *pdev)
{
	int ret;
	
	struct resource *res1;
	char *name;
	int minor;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//1，申请全局设备对象空间
	led_dev = kzalloc(sizeof(*led_dev), GFP_KERNEL);
	if(!led_dev){
		printk("kzalloc error");
		return -ENOMEM;
	}
		
	//获取device_node结点
	led_dev->np = pdev->dev.of_node;
	of_property_read_u32(led_dev->np, "mode_data", &led_dev->mode_data);
	of_property_read_u32(led_dev->np, "mode_clear", &led_dev->mode_clear);
	of_property_read_u32(led_dev->np, "odr", &led_dev->odr);
	of_property_read_u32(led_dev->np, "shift", &led_dev->shift);
	
	of_property_read_string(led_dev->np, "dev-name", (const char * *) &name);
	of_property_read_u32(led_dev->np, "minor", &minor);
	//2,初始化杂项设备对象
	led_dev->misc.fops  =  &led_fops;  //设备操作对象地址
	led_dev->misc.minor = minor;          //次设备号
	led_dev->misc.name = name;    //设备结点名称

	//3,注册杂项设备对象
	ret = misc_register(&led_dev->misc);
	if(ret < 0){
		printk("misc_register error\n");
		goto err_kfree;
	}

	//获取平台资源
	res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0); //资源编号从0开始，表示同类型资源编号
	if(!res1){
		printk("platform_get_resource error\n");
		goto err_misc_deregister;
	}
	printk("res1->start = %x\n",res1->start);


	//4,硬件初始化
   	led_dev->gpioz = ioremap(res1->start,res1->end-res1->start+1);    
	if(!led_dev->gpioz){
			printk("ioremap error\n");
			ret = PTR_ERR(led_dev->gpioz);
			goto err_misc_deregister;
		}
	
	
	return 0;
err_misc_deregister:
	misc_deregister(&led_dev->misc);
err_kfree:
	kfree(led_dev);
	return ret;

}
int led_pdrv_remove(struct platform_device *pdev)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	iounmap(led_dev->gpioz);
	misc_deregister(&led_dev->misc);
	kfree(led_dev);

	return 0;
}

const struct platform_device_id  led_id_table[] = {
	{"mp157_led",0x1234},
	{"mp158_led",0x2345},
	{"mp159_led",0x3456},
};
const struct of_device_id	led_of_match_table[] = {
	{.compatible = "stm32mp157,led_test1"},
};


//1,实例化pdrv对象
struct platform_driver  led_pdrv = {
	.probe 	 	=  led_pdrv_probe,
	.remove		=  led_pdrv_remove,
	.driver 	=	{
		.name 	=	"stm32mp157_led",    //必须要赋值：ls /sys/bus/platform/drivers/stm32mp157_led/
		.of_match_table = led_of_match_table,
	},
	.id_table 	=	led_id_table,
};

static int __init led_pdrv_init(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	// 2, 注册pdrv对象
	return platform_driver_register(&led_pdrv);
}
static void __exit led_pdrv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	platform_driver_unregister(&led_pdrv);
}


//4,声明与认证
module_init(led_pdrv_init);
module_exit(led_pdrv_exit);
MODULE_LICENSE("GPL");
