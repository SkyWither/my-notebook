#include <linux/init.h>
#include <linux/module.h>
#include "stm32mp157_gpio.h"
#include <linux/platform_device.h>
#include "led_plat.h"

//定义资源
struct resource led_resource[] = {
	[0] = {
		.start 	=	GPIOZ,
		.end    =   GPIOZ + 24 - 1,
		.flags  =   IORESOURCE_MEM, 
	},

	//以下为资源测试
	[1] = {
		.start 	=	120,
		.end    =   120,
		.flags  =   IORESOURCE_IRQ, 
	},
	[2] = {
		.start 	=	GPIOA,
		.end    =   GPIOA + 24 - 1,
		.flags  =   IORESOURCE_MEM, 
	},
};
void	led_pdev_release(struct device *dev)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
}

//平台自定义数据
struct led_platdata led_pdata = {
	.name		=	"led02", 				 //设备结点名称
	.mode_clear =	0x3f,	 //0x3f
	.mode_data  =   0x15,	//0x15
	.shift 		=	5,		//5
	.odr		=	0x7,			//0x7
	.minor		=	8,					//次设备号
};

//1,实例化pdev对象
struct platform_device  led_pdev={
	.name	=	"mp157_led",
	.num_resources = ARRAY_SIZE(led_resource),
	.resource = led_resource,
	.dev = {
		.release  = led_pdev_release,
		.platform_data = &led_pdata,
	},
};

static int __init led_pdev_init(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	// 2,注册pdev
	return platform_device_register(&led_pdev);
}
static void __exit led_pdev_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	platform_device_unregister(&led_pdev);
}

module_init(led_pdev_init);
module_exit(led_pdev_exit);
MODULE_LICENSE("GPL");



