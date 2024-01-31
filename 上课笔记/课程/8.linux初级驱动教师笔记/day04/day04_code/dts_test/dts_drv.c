#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/mod_devicetable.h>
int dts_pdrv_probe(struct platform_device *pdev)
{
	struct device_node *np,*sun;
	struct resource *res1,*res2;
	struct property *prop;
	const char *s;
	char *name;
	u32 u,age;
	const __be32 *p;
	u8 b[4];
	int i;
	printk("-----------^_^ %s----------------\n",__FUNCTION__);
	
	//获取设备树中结点属性

	//1，获取设备树结点对应的device_node结点
	np = pdev->dev.of_node;

	//2,获取各种属性值

	//2.1 获取平台资源
	res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	printk("res1->start = %x,res2->start = %x\n",res1->start,res2->start);

	//2.2 获取字符串数组
	of_property_for_each_string(np, "string_array", prop, s){
		printk("s = %s\n",s);
	}

	//2.3 获取整数数组
	of_property_for_each_u32(np, "ages", prop, p, u){
		printk("u = %d\n",u);
	}
	
	//2.4 获取二进制数组
	of_property_read_u8_array(np, "bin", b, 4);
	for(i = 0; i < 4; i++)
		printk("%x\n",b[i]);

	//2.5 获取子结点
	sun = of_find_node_by_name(np, "sun");
	of_property_read_string(sun, "sun_name", (const char * *)&name);
	printk("name = %s\n",name);
	of_property_read_u32(sun, "age", &age);
	printk("age = %d\n",age);

	return 0;
	
	
}
int drs_pdrv_remove(struct platform_device *pdev)
{
	printk("-----------^_^ %s----------------\n",__FUNCTION__);

	return 0;
}

const struct of_device_id	dts_of_match_table[] = {
	{.compatible = "stm32mp157,test_dev"},
};
// 1，实例化pdrv对象
struct platform_driver  dts_pdrv = {
	.probe		=		dts_pdrv_probe,
	.remove		=		drs_pdrv_remove,
	.driver 	=	{
		.name	=	"dts_pdrv_test",
		.of_match_table	=	dts_of_match_table,
	},
};

static int __init dts_test_init(void)
{
	printk("-----------^_^ %s----------------\n",__FUNCTION__);
	//2，注册pdrv 
	return platform_driver_register(&dts_pdrv);
}

static void __exit dts_test_exit(void)
{
	printk("-----------^_^ %s----------------\n",__FUNCTION__);
	platform_driver_unregister(&dts_pdrv);
}

module_init(dts_test_init);
module_exit(dts_test_exit);
MODULE_LICENSE("GPL");
