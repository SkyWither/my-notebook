//1,包含头文件
#include <linux/init.h>
#include <linux/module.h>
#include "fun.h"

char *name = "Jack";
int age = 10;
int a = 1,b=2;

//2,实现模块加载(入口)函数
static int __init hello_drv_init(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	printk("name = %s,age = %d\n",name,age);
	printk("%d + %d = %d\n",a,b,myadd(a, b));
	printk("%d - %d = %d\n",a,b,mysub(a, b));
	return 0;
}
//3,实现模块卸载(出口)函数
static void __exit hello_drv_exit(void)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
}

//声明变量，则可以在加载模块时传递参数给变量
module_param(name, charp, 0644);
module_param(age, int, 0644);
module_param(a, int, 0644);
module_param(b, int, 0644);





//4,声明与认证
module_init(hello_drv_init);
module_exit(hello_drv_exit);
MODULE_LICENSE("GPL");
