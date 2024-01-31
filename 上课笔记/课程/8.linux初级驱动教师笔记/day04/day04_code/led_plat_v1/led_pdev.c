#include <linux/init.h>
#include <linux/module.h>
#include "stm32mp157_gpio.h"
#include <linux/platform_device.h>


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


//1,实例化pdev对象
struct platform_device  led_pdev={
	.name	=	"mp157_led",
	.num_resources = ARRAY_SIZE(led_resource),
	.resource = led_resource,
	.dev = {
		.release  = led_pdev_release,
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



