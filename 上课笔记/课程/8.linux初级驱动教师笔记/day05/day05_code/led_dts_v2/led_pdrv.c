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
#include <linux/of_gpio.h>
#include <linux/gpio.h>


//全局设备对象类型
struct mp157_led{
	struct miscdevice  misc;
	struct device_node *np;
	int gpioz_5;
	int gpioz_6;
	int gpioz_7;
	
};

struct mp157_led  *led_dev;



//4,实现设备操作接口函数
int led_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

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

	gpio_request(led_dev->gpioz_5, "gpioz_5");
	gpio_request(led_dev->gpioz_6, "gpioz_6");
	gpio_request(led_dev->gpioz_7, "gpioz_7");
	//判断应用传递的数据 1---开灯，0 --- 关灯
	if(value){
		//开灯
		gpio_direction_output(led_dev->gpioz_5, 1);
		gpio_direction_output(led_dev->gpioz_6, 1);
		gpio_direction_output(led_dev->gpioz_7, 1);
	}else{
		//关灯
		gpio_direction_output(led_dev->gpioz_5, 0);
		gpio_direction_output(led_dev->gpioz_6, 0);
		gpio_direction_output(led_dev->gpioz_7, 0);
	}
	gpio_free(led_dev->gpioz_5);
	gpio_free(led_dev->gpioz_6);
	gpio_free(led_dev->gpioz_7);
	return size;
}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	gpio_request(led_dev->gpioz_5, "gpioz_5");
	gpio_request(led_dev->gpioz_6, "gpioz_6");
	gpio_request(led_dev->gpioz_7, "gpioz_7");
	gpio_direction_output(led_dev->gpioz_5, 0);
	gpio_direction_output(led_dev->gpioz_6, 0);
	gpio_direction_output(led_dev->gpioz_7, 0);
	gpio_free(led_dev->gpioz_5);
	gpio_free(led_dev->gpioz_6);
	gpio_free(led_dev->gpioz_7);
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

	//获取led对应的gpio编号
	led_dev->gpioz_5 =  of_get_gpio(led_dev->np, 0);
	led_dev->gpioz_6 =  of_get_gpio(led_dev->np, 1);
	led_dev->gpioz_7 =  of_get_gpio(led_dev->np, 2);
	
	
	
	return 0;
err_kfree:
	kfree(led_dev);
	return ret;

}
int led_pdrv_remove(struct platform_device *pdev)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
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
	{.compatible = "stm32mp157,led_test2"},
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
