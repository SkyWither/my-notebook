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
#include <linux/interrupt.h>


//全局设备对象类型
struct mp157_key{
	struct miscdevice  misc;
	struct device_node *np;
	int irqno;	
	struct gpio_desc * gpiof_7;
};

struct mp157_key  *key_dev;



//4,实现设备操作接口函数
int key_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

	return 0;
}

int key_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

	return 0;
}

const struct file_operations key_fops = {
	.open	=	key_drv_open,
	.release = key_drv_close,
};

//实现中断处理函数
irqreturn_t key_irq_fun(int irqno, void * dev_id)
{
	int value;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

	//获取gpio引脚状态
	value = gpiod_get_value(key_dev->gpiof_7);

	//判断key按下还是松开
	if(value){
		//松开
		printk("released key1\n");
	}else{
		//按下
		printk("pressed key1\n");
	}	
	return IRQ_HANDLED;
}

//实现probe
int key_pdrv_probe(struct platform_device *pdev)
{
	int ret;
	char *name;
	int minor;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	//1，申请全局设备对象空间
	key_dev = kzalloc(sizeof(*key_dev), GFP_KERNEL);
	if(!key_dev){
		printk("kzalloc error");
		return -ENOMEM;
	}
	
	//获取device_node结点
	key_dev->np = pdev->dev.of_node;
	
	of_property_read_string(key_dev->np, "dev-name", (const char * *) &name);
	of_property_read_u32(key_dev->np, "minor", &minor);
	//2,初始化杂项设备对象
	key_dev->misc.fops  =  &key_fops;  //设备操作对象地址
	key_dev->misc.minor = minor;          //次设备号
	key_dev->misc.name = name;    //设备结点名称

	//3,注册杂项设备对象
	ret = misc_register(&key_dev->misc);
	if(ret < 0){
		printk("misc_register error\n");
		goto err_kfree;
	}

	//获取key1对应的gpio引脚编号
	key_dev->gpiof_7 = devm_gpiod_get_index(&pdev->dev, "key", 0, GPIOD_OUT_HIGH);

	//获取key1对应的虚拟中断号
	key_dev->irqno = gpiod_to_irq(key_dev->gpiof_7);

	//申请中断
	ret = request_irq(key_dev->irqno, key_irq_fun, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "key1", NULL);
	if(ret){
		printk("request_irq error");
		goto err_misc_deregister;
	}
	
	return 0;
err_misc_deregister:
	misc_deregister(&key_dev->misc);
err_kfree:
	kfree(key_dev);
	return ret;

}
int key_pdrv_remove(struct platform_device *pdev)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	free_irq(key_dev->irqno, NULL);
	misc_deregister(&key_dev->misc);
	kfree(key_dev);

	return 0;
}

const struct of_device_id	key_of_match_table[] = {
	{.compatible = "stm32mp157,key_test3"},
};


//1,实例化pdrv对象
struct platform_driver  key_pdrv = {
	.probe 	 	=  key_pdrv_probe,
	.remove		=  key_pdrv_remove,
	.driver 	=	{
		.name 	=	"stm32mp157_key",    //必须要赋值：ls /sys/bus/platform/drivers/stm32mp157_key/
		.of_match_table = key_of_match_table,
	},
};

static int __init key_pdrv_init(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	// 2, 注册pdrv对象
	return platform_driver_register(&key_pdrv);
}
static void __exit key_pdrv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	platform_driver_unregister(&key_pdrv);
}


//4,声明与认证
module_init(key_pdrv_init);
module_exit(key_pdrv_exit);
MODULE_LICENSE("GPL");
