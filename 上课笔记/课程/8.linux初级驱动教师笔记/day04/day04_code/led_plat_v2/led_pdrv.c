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

#include "stm32mp157_gpio.h"
#include "led_plat.h"



//全局设备对象类型
struct mp157_led{
	struct miscdevice  misc;
	gpio_t *gpioz;
	struct led_platdata *pd;
};

struct mp157_led  *led_dev;



//4,实现设备操作接口函数
int led_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//将gpio设置为输出模式
	led_dev->gpioz->MODER &=  ~(led_dev->pd->mode_clear << led_dev->pd->shift *2);
	led_dev->gpioz->MODER |= led_dev->pd->mode_data << led_dev->pd->shift *2;

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
		led_dev->gpioz->ODR |= led_dev->pd->odr << led_dev->pd->shift;
	}else{
		//关灯
		led_dev->gpioz->ODR &= ~(led_dev->pd->odr << led_dev->pd->shift);
	}

	return size;
}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	led_dev->gpioz->ODR &= ~(led_dev->pd->odr << led_dev->pd->shift);
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
	struct resource *res1,*res2,*res3;
	int irqno;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//1，申请全局设备对象空间
	led_dev = kzalloc(sizeof(*led_dev), GFP_KERNEL);
	if(!led_dev){
		printk("kzalloc error");
		return -ENOMEM;
	}

	//获取平台自定义数据
	led_dev->pd = pdev->dev.platform_data;
	
	//2,初始化杂项设备对象
	led_dev->misc.fops  =  &led_fops;  //设备操作对象地址
	led_dev->misc.minor = led_dev->pd->minor;          //次设备号
	led_dev->misc.name =  led_dev->pd->name;    //设备结点名称

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
	//获取平台资源
	res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1); //资源编号从0开始，表示同类型资源编号
	if(!res2){
		printk("platform_get_resource error\n");
		goto err_misc_deregister;
	}
	printk("res2->start = %x\n",res2->start);
	//获取平台资源
	res3 = platform_get_resource(pdev, IORESOURCE_IRQ, 0); //资源编号从0开始，表示同类型资源编号
	if(!res3){
		printk("platform_get_resource error\n");
		goto err_misc_deregister;
	}
	printk("res3->start = %d,res3->end = %d\n",res3->start,res3->end);

	//中断资源另一种获取方式
	irqno = platform_get_irq(pdev, 0);
	printk("irqno = %d\n",irqno);

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


//1,实例化pdrv对象
struct platform_driver  led_pdrv = {
	.probe 	 	=  led_pdrv_probe,
	.remove		=  led_pdrv_remove,
	.driver 	=	{
		.name 	=	"stm32mp157_led",    //必须要赋值：ls /sys/bus/platform/drivers/stm32mp157_led/
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
