//1,包含头文件
#include <linux/init.h>
#include <linux/module.h>


//2,实现模块加载(入口)函数
static int __init hello_drv_init(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	return 0;
}
//3,实现模块卸载(出口)函数
static void __exit hello_drv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
}

//4,声明与认证
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");
