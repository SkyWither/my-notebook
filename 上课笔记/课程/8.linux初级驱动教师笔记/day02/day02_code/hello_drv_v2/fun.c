#include <linux/init.h>
#include <linux/module.h>


static int myadd(int a,int b)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	return a+b;
}
EXPORT_SYMBOL(myadd);

static int mysub(int a,int b)
{
	printk("-----------^_^ %s-------------\n",__FUNCTION__);
	return a-b;
}
EXPORT_SYMBOL(mysub);

MODULE_LICENSE("GPL");


