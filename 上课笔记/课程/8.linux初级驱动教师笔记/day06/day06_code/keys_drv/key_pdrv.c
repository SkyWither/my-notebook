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
#include <linux/input-event-codes.h>

//定义按键信息类型
struct mp157_key_info{
	char name[50];
	int code;
	struct gpio_desc * gpio;
	int irqno;
	int flags;
};

struct mp157_key_info key_set[] = {
	[0] = {
		.name = "key1",
		.code = KEY_1,
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
	},
	[1] = {
		.name = "key2",
		.code = KEY_2,
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
	},
	[2] = {
		.name = "key3",
		.code = KEY_3,
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
	},

};


//定义按钮数据类型
struct mp157_key_data{
	int code;
	int value;     //1 ---按下 ，0---松开
};

//全局设备对象类型
struct mp157_key{
	struct miscdevice  misc;
	struct device_node *np;	
	struct mp157_key_data kdata;
	struct wait_queue_head  wq; 
	int have_data;    //有数据---1,没有数据---0
};

struct mp157_key  *key_dev;



//4,实现设备操作接口函数
int key_drv_open(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	key_dev->have_data = 0;
	return 0;
}

int key_drv_close(struct inode *inode, struct file *filp)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

	return 0;
}

ssize_t key_drv_read(struct file *filp, char __user *buf, size_t size, loff_t *flags)
{
	int ret;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

	//如果应用层open时加入O_NONBLOCK,且此时没有数据可读，进程立即返回
	if((filp->f_flags & O_NONBLOCK)  && (!key_dev->have_data))
		return -EAGAIN;

	//有数据则读数据，没有数据则阻塞
	wait_event_interruptible(key_dev->wq, key_dev->have_data);  //have_data为真，则返回
	
	ret = copy_to_user(buf, &key_dev->kdata, size);
	if(ret > 0){
		printk("copy_to_user error");
		return -EINVAL;			
	}

	key_dev->have_data = 0;
	
	return size;
}


const struct file_operations key_fops = {
	.open	=	key_drv_open,
	.release = key_drv_close,
	.read	=	key_drv_read,
};

//实现中断处理函数
irqreturn_t key_irq_fun(int irqno, void * dev_id)
{
	int value;
	struct mp157_key_info *kf;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);

	//获取当前触发中断的按键信息
	kf = (struct mp157_key_info *)dev_id;

	//获取gpio引脚状态
	value = gpiod_get_value(kf->gpio);

	key_dev->kdata.code = kf->code;
	
	//判断key按下还是松开
	if(value){
		//松开
		printk("released %s\n",kf->name);
		key_dev->kdata.value = 0;
		
	}else{
		//按下
		printk("pressed %s\n",kf->name);
		key_dev->kdata.value = 1;
	}	

	key_dev->have_data = 1;
	wake_up_interruptible(&key_dev->wq); //唤醒阻塞的进程
	return IRQ_HANDLED;
}

//实现probe
int key_pdrv_probe(struct platform_device *pdev)
{
	int ret,i;
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

	for(i =0; i < ARRAY_SIZE(key_set);i++){
		//获取key对应的gpio引脚编号
		key_set[i].gpio = devm_gpiod_get_index(&pdev->dev, "key", i, GPIOD_OUT_HIGH);

		//获取key1对应的虚拟中断号
		key_set[i].irqno = gpiod_to_irq(key_set[i].gpio);

		//申请中断
		ret = request_irq(key_set[i].irqno, key_irq_fun, key_set[i].flags, key_set[i].name, &key_set[i]);
		if(ret){
			printk("request_irq error");
			goto err_misc_deregister;
		}
	}

	//初始化等待队列头
	init_waitqueue_head(&key_dev->wq);
	
	return 0;
err_misc_deregister:
	misc_deregister(&key_dev->misc);
err_kfree:
	kfree(key_dev);
	return ret;

}
int key_pdrv_remove(struct platform_device *pdev)
{
	int i;
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	for(i =0; i < ARRAY_SIZE(key_set);i++)
		free_irq(key_set[i].irqno, &key_set[i]);
	misc_deregister(&key_dev->misc);
	kfree(key_dev);

	return 0;
}

const struct of_device_id	key_of_match_table[] = {
	{.compatible = "stm32mp157,keys_test"},
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
