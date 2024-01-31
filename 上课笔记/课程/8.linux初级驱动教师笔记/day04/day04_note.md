#### 一，平台总线驱动

##### 1，概念和作用

```
1》总线(高速公路)
	在linux中，总线分为两种类型
	物理总线：i2c，spi，usb等
	虚拟总线：platform
	
2》平台总线--分离思想
	在编写驱动时：将驱动中通用的操作与硬件数据分离
	在加载驱动时，再将其匹配，组成一个完整的驱动程序
	
3》作用：操作和数据分离
	减少了编写重复的驱动代码
	有利于平台的升级。

编写思路：面向对象方式

platform driver ----实现相似的操作
-------------------------------------
platform_bus ---- 匹配pdrv 与 pdev
-------------------------------------
platform device --- 封装硬件数据

```

##### 2,了解相关的结构体

###### 2.1 struct platform_driver

```c
//封装通用操作
struct platform_driver {
	int (*probe)(struct platform_device *);		//实现传统驱动开发中加载函数中的代码
	int (*remove)(struct platform_device *);    //实现传统驱动开发中卸载函数中的代码
	void (*shutdown)(struct platform_device *);
	int (*suspend)(struct platform_device *, pm_message_t state);
	int (*resume)(struct platform_device *);
	struct device_driver driver;    //父类
	const struct platform_device_id *id_table;   //用于匹配
	bool prevent_deferred_probe;
};

//注册和注销
int platform_driver_register(struct platform_driver * drv);
void platform_driver_unregister(struct platform_driver *);
```

###### 2.2 struct  bus_type 

```C
//总线类型
struct bus_type {
	const char		*name;
	const char		*dev_name;
	struct device		*dev_root;
	const struct attribute_group **bus_groups;
	const struct attribute_group **dev_groups;
	const struct attribute_group **drv_groups;

	int (*match)(struct device *dev, struct device_driver *drv); //匹配函数
	int (*uevent)(struct device *dev, struct kobj_uevent_env *env);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
	void (*shutdown)(struct device *dev);

	int (*online)(struct device *dev);
	int (*offline)(struct device *dev);

	int (*suspend)(struct device *dev, pm_message_t state);
	int (*resume)(struct device *dev);

	int (*num_vf)(struct device *dev);

	int (*dma_configure)(struct device *dev);

	const struct dev_pm_ops *pm;

	const struct iommu_ops *iommu_ops;

	struct subsys_private *p;
	struct lock_class_key lock_key;

	bool need_parent_lock;
};

//注册和注销
extern int __must_check bus_register(struct bus_type *bus);
extern void bus_unregister(struct bus_type *bus);
```

###### 2.3 struct platform_device

```C
//平台设备对象类型
struct platform_device {
	const char	*name;         //名称，用于与pdrv匹配
	int		id;				  //一般为-1
	bool		id_auto;
	struct device	dev;     //父类
	u64		platform_dma_mask;
	u32		num_resources;        //资源个数
	struct resource	*resource;    //资源

	const struct platform_device_id	*id_entry;
	char *driver_override; /* Driver name to force a match */

	/* MFD cell pointer */
	struct mfd_cell *mfd_cell;

	/* arch specific additions */
	struct pdev_archdata	archdata;
};
//注册和注销
extern int platform_device_register(struct platform_device *);
extern void platform_device_unregister(struct platform_device *);
```

###### 2.4 struct resource

```C
//平台资源---有两种类型资源：地址(内存)资源和中断资源
struct resource {
	resource_size_t start;   //如果是地址资源，start表示起始地址，如果是中断资源，start表示中断号
	resource_size_t end;    //如果是地址资源，end表示内存最后一个字节地址，如果是中断资源，start表示中断号
	const char *name;   
	unsigned long flags;   //资源种类:地址资源IORESOURCE_MEM  中断资源IORESOURCE_IRQ
	unsigned long desc;
	struct resource *parent, *sibling, *child;
};
```

##### 3，实现平台总线驱动

###### 3.1  搭建框架

```sh
//参考代码：led_plat_v1
开发板中加载：
//加载
[root@fsmp1a drv_modules]# insmod led_pdev.ko
[17861.995259] ----------^_^ led_pdev_init--------------
[root@fsmp1a drv_modules]# insmod led_pdrv.ko
[17867.198317] ---------^_^ led_pdrv_init--------------
[17867.202169] ---------^_^ led_pdrv_probe--------------		//如果匹配成功，则会调用probe函数
[17867.207260] res1->start = 54004000
[17867.210265] res2->start = 50002000
[17867.213659] res3->start = 120,res3->end = 120    
[root@fsmp1a drv_modules]# ls /sys/bus/platform/drivers/fsmp157_led/   //同时在此目录中创建pdev相关信息
bind       module     mp157_led  uevent     unbind           //匹配成功，创建pdev对象对应的信息mp157_led

//卸载
[root@fsmp1a drv_modules]# rmmod led_pdev.ko
[   28.631364] irqno = 120
[  231.759240] ----------^_^ led_pdev_exit--------------
[  231.765388] ---------^_^ led_pdrv_remove--------------
[  231.770956] ------------[ cut here ]------------
[  231.775044] WARNING: CPU: 0 PID: 150 at drivers/base/core.c:1103 device_release+0x94/0x98
[  231.783289] Device 'mp157_led' does not have a release() function, it is broken and must be fixed. See Documentation/kobject.txt.
[  231.795001] Modules linked in: led_pdev(OE-) led_pdrv(OE)
[  231.800307] CPU: 0 PID: 150 Comm: rmmod Tainted: G           OE     5.4.31 #2
[  231.807391] Hardware name: STM32 (Device Tree Support)
[  231.812563] [<c01124dc>] (unwind_backtrace) from [<c010d784>] (show_stack+0x10/0x14)
[  231.820285] [<c010d784>] (show_stack) from [<c0b0aa54>] (dump_stack+0xb0/0xc4)
[  231.827512] [<c0b0aa54>] (dump_stack) from [<c01259d4>] (__warn+0xd0/0xf8)
[  231.834385] [<c01259d4>] (__warn) from [<c0125db0>] (warn_slowpath_fmt+0x98/0xc4)
[  231.841873] [<c0125db0>] (warn_slowpath_fmt) from [<c066edf4>] (device_release+0x94/0x98)
[  231.850052] [<c066edf4>] (device_release) from [<c0b0f72c>] (kobject_put+0xb8/0x214)
[  231.857798] [<c0b0f72c>] (kobject_put) from [<c01bcc5c>] (sys_delete_module+0x134/0x228)
[  231.865889] [<c01bcc5c>] (sys_delete_module) from [<c0101000>] (ret_fast_syscall+0x0/0x54)
[  231.874144] Exception stack(0xd189ffa8 to 0xd189fff0)
[  231.879197] ffa0:                   000307e4 5f64656c be8f5b88 00000880 00000000 be8f5e18
[  231.887380] ffc0: 000307e4 5f64656c 76656470 00000081 00000000 00000000 b6f46000 00000000
[  231.895554] ffe0: be8f5b80 be8f5b70 000306b4 b6e7b020
[  231.900904] ---[ end trace a388f547fbb5594d ]---
```
```C
//上面错误的解决方法如下：
struct platform_device  led_pdev = {
	.name   =  "fsmp1_led",
	.id 	=	-1,
	.num_resources	=	ARRAY_SIZE(led_resource),
	.resource = led_resource,
	.dev = {
		.release 	=	led_plat_release,
	},
};
//实现release函数
void	led_plat_release(struct device *dev)
{
	printk("--------^_^ %s---------\n",__FUNCTION__);
}
```
```sh
//再次编译，卸载
[root@fsmp1a drv_modules]# rmmod led_pdev.ko
[  456.939912] ----------^_^ led_pdev_exit--------------
[  456.943641] ---------^_^ led_pdrv_remove--------------
[  456.949120] --------^_^ led_plat_release---------
[root@fsmp1a drv_modules]# rmmod led_pdrv.ko
[  476.684906] ---------^_^ led_pdrv_exit--------------
```

###### 3.2 获取平台资源，实现probe

```C
//获取资源
struct resource *platform_get_resource(struct platform_device *dev,
				       unsigned int type, unsigned int num)
//参数1 ---- pdev对象指针
//参数2 ---- 资源的类型
//参数3 ---- 同类型资源的编号，此编号从0开始
//返回值 ----成功：资源指针，失败：NULL

例如：
	//获取平台资源
	res1 = platform_get_resource(pdev, IORESOURCE_MEM,0);  //同类型资源编号从0开始
	printk("res1->start = %x\n",res1->start);
	res2 = platform_get_resource(pdev, IORESOURCE_MEM,1);  //同类型资源编号从0开始
	printk("res2->start = %x\n",res2->start);

	res3 = platform_get_resource(pdev, IORESOURCE_IRQ,0);  //获取第一个中断资源
	printk("res3->start = %d,res3->end = %d\n",res3->start,res3->end);

	irqno = platform_get_irq(pdev, 0);  //第二种获取中断资源方式
	printk("irqno = %d\n",irqno);
```

###### 3.2  平台自定义数据

```C
//参考代码:led_plat_v2
1》定义平台数据类型  ----//头文件中定义
    struct led_platdata{
        char * name;             // 设备节点名称
        int  minor;              //次设备号
        int  mreg_clear;        //0x3f
        int  mreg_data;			//0x15
        int  dreg;  			//0x7
        int shift;				//5
    };
2》在pdev中封装自定义数据
	//实例化平台自定义数据
 	struct led_platdata  led_pdata = {
		.name   =  "led2",			 // 设备节点名称
		.minor	=	7, 			 //次设备号
		.mreg_clear	=	0x3f,		//0x3f
		.mreg_data	=	0x15,		//0x15
		.dreg		=	0x7,			//0x7
		.shift		=	5,			//5
	};


    // 1，实例化pdev对象
   struct platform_device  led_pdev = {
        .name	=	"mp157_led",
        .id 	=	-1,
        .resource	=	led_pdev_res,
        .num_resources	=	ARRAY_SIZE(led_pdev_res),
        .dev = {
            .release 	=	led_plat_release,
            .platform_data	=	&led_pdata,
        },
    };
    
3》在pdrv的probe中获取平台自定义数据
	//获取平台自定义数据
	led_dev->pd = pdev->dev.platform_data;
	
4》在pdrv中自定义数据实现设备操作接口函数
	int led_drv_open(struct inode *inode, struct file *filp)
    {
        printk("---------^_^ %s--------------\n",__FUNCTION__);
        //将gpioz_5,gpioz_6,gpioz_7设置为输出模式
        led_dev->gpioz->MODER &= ~(led_dev->pd->mreg_clear << (led_dev->pd->shift*2));
        led_dev->gpioz->MODER |= led_dev->pd->mreg_data << (led_dev->pd->shift*2);	
        return 0;
    }
  
//在开发板测试：
[root@fsmp1a drv_modules]# insmod led_pdev.ko
[ 1770.763541] ----------^_^ led_pdev_init--------------
[root@fsmp1a drv_modules]# insmod led_pdrv.ko
[ 1774.939745] ---------^_^ led_pdrv_init--------------
[ 1774.943597] ---------^_^ led_pdrv_probe--------------
[ 1774.948720] res1->start = 54004000
[ 1774.951689] res2->start = 50002000
[ 1774.955082] res3->start = 120,res3->end = 120
[ 1774.959507] irqno = 120
[root@fsmp1a drv_modules]# ./test_led
[ 1777.912844] ---------^_^ led_drv_open--------------
[ 1777.916294] ---------^_^ led_drv_ioctl--------------
[ 1778.921567] ---------^_^ led_drv_ioctl--------------
[ 1779.925201] ---------^_^ led_drv_ioctl--------------
[ 1780.928879] ---------^_^ led_drv_ioctl--------------
[ 1781.932475] ---------^_^ led_drv_ioctl--------------
[ 1782.936110] ---------^_^ led_drv_ioctl--------------
[ 1783.939793] ---------^_^ led_drv_ioctl--------------
[ 1784.943418] ---------^_^ led_drv_ioctl--------------
^C[ 1785.656495] ---------^_^ led_drv_close--------------

```

#### 二，设备树

##### 1，概念

```
在传统 Linux 内核中，ARM 架构的板极硬件细节过多地被硬编码在 arch/arm/plat-xxx 和arch/arm/mach-xxx，比如板上的 platform 设备、resource、i2c_board_info、spi_board_info 以及各种硬件的 platform_data，这些板级细节代码对内核来讲只不过是垃圾代码。而采用Device Tree 后，许多硬件的细节可以直接透过它传递给 Linux，而不再需要在 kernel 中进行大量的冗余编码。导致 ARM 的 merge 工作量较大。

之后经过linux团队一些讨论，对 ARM 平台的相关 code 做出如下相关规范调整，这个也正是引入DTS 的原因。
1、ARM 的核心代码仍然保存在 arch/arm 目录下
2、ARM SoC core architecture code 保存在 arch/arm 目录下
3、ARM SOC 的周边外设模块的驱动保存在 drivers 目录下
4、ARM SOC 的特定代码在 arch/arm/mach-xxx 目录下
5、ARM SOC board specific 的代码被移除，由 DeviceTree 机制来负责传递硬件拓扑和硬件资源信息。

本质上，Device Tree 改变了原来用 code 方式将硬件配置信息嵌入到内核代码的方法，改用 bootloader 传递一个 DB 的形式。
对于嵌入式系统，在系统启动阶段，bootloader 会加载内核并将控制权转交给内核。

在 devie tree 中，可描述的信息包括：
    1、CPU 的数量和类别
    2、内存基地址和大小
    3、总线和桥
    4、外设连接
    5、中断控制器和中断的使用情况
    6、GPIO 控制器和 GPIO 使用情况
    7、clock 控制器和 clock 使用情况
    
设备树基本就是一棵电路板上的 CPU、总线、设备组成的树，Bootloader 会将这棵树传递给内核，然后内核来识别这棵树，并根据它展开出 Linux 内核中的 platform_device、i2c_client、spi_device 等设备，而这些设备用到的内存、IRQ 等资源，也被传递给内核，内核会将这些资源绑定给展开的相应设备。

Linux 内核从 3.x 开始引入设备树的概念，用于实现驱动代码与设备信息相分离。在设备树出现以前，所有关于设备的具体信息都要写在驱动里，一旦外围设备变化，驱动代码就要重写。引入了设备树之后，驱动代码只负责处理驱动的逻辑，而关于设备的具体信息存放到设备树文件中，这样，如果只是硬件接口信息的变化而没有驱动逻辑的变化，驱动开发者只需要修改设备树文件信息，不需要改写驱动代码。比如在 ARM Linux 内，一个.dts(de
vicetree source)文件对应一个 ARM 的 machine，一般放置在内核的"arch/arm/boot/dts/"目录内，比如 stmp1a-dk1 参考板的板级设备树文件就是"arch/arm/boot/dts/ stm32mp157a-dk1.dts"。这个文件可以通过 make dtbs 命令编译成二进制的.dtb 文件供内核驱动使用。
```

##### 2，设备树语法

###### 2.1 几个相关的文件

```
//在linux内核中，设备树文件一般存放在：arch/arm/boot/dts/
1》设备树源文件
dts：设备树源文件----硬件的相应信息都会写在.dts 为后缀的文件中，每一款硬件可以单独写一份例如stm32mp157a-dk1.dts，一般在 Linux 源码中存在大量的 dts 文件，对于 arm 架构可以在 arch/arm/boot/dts 找到相应的 dts，一个 dts 文件对应一个 ARM 的 machie。 
2》设备树头文件
  dtsi：设备树头文件---值得一提的是，对于一些相同的 dts 配置可以抽象到 dtsi 文件中，然后类似于 C 语言的方式可以 include 到 dts 文件中，对于同一个节点的设置情况，dts 中的配置会覆盖 dtsi 中的配置。
3》设备树编译工具 
 dtc：是编译dts的工具，可以在 Ubuntu 系统上通过指令 apt-get install device-tree-compiler 安装 dtc 工具，不过在内核源码 scripts/dtc 路径下已经包含了 dtc 工具；
4》设备树编译生成的文件
 dtb(Device Tree Blob) ：设备树源文件编译后生成的文件 ----- dts 经过 dtc 编译之后会得到 dtb 文件，dtb 通过 Bootloader 引导程序加载到内核。所以 Bootloader 需要支持设备树才行；Kernel 也需要加入设备树的支持；
  
  设备树的结构：
  
  /dts-v1/;
  / {
    node1 {
        a-string-property = "A string";
        a-string-list-property = "first string", "second string";
        // hex is implied in byte arrays. no '0x' prefix is required
        a-byte-data-property = [01 23 34 56];
        child-node1 { 
        	first-child-property;
         	second-child-property = <1>;
             a-string-property = "Hello, world";
     	};
         child-node2 {
         };
 	};
     node2 {
            an-empty-property;
            a-cell-property = <1 2 3 4>; /* each number (cell) is a uint32 */
                child-node1 {
                };
        };
    };
	device tree 的基本单元是 node。这些 node 被组织成树状结构，除了 root node，每个node 都只有一个 parent。一个 device tree 文件中只能有一个 root node。每个 node 中包含了若干的 property/value 来描述该 node 的一些特性。每个 node 用节点名字（node name）标识，节点名字的格式是 node-name@unit-address。如果该 node 没有 reg 属性（后面会描述这个property），那么该节点名字中必须不能包括@和 unit-address。unit-address 的具体格式是和设备挂在那个 bus 上相关。例如对于 cpu，其 unit-address 就是从 0 开始编址，以此加一。而具体的设备，例如以太网控制器，其 unit-address 就是寄存器地址。root node 的 node name 是确
定的，必须是“/”。
也就是说设备树源文件的结构为: 
	1 个 root 节点”/”; 
    root 节点下面含一系列子节点，“node1” and “node2” 
    节点 node1 和下又含有一系列子节点，“child-node1” and “child-node2” 
    各个节点都有一系列属性
        这些属性可能为空，如 an-empty-property
        可能为字符串，如 a-string-property
		可能为字符串树组，如 a-string-list-property
		可能为 Cells（由 u32 整数组成），如 second-child-property
```

###### 2.2 设备树文件编译和反编译

```
在一个dts文件中个，经常会包含许多dtsi文件，有时候dtsi会嵌套很深，此时不利于我们对设备树文件的阅读和理解，这是可以将编译好的dtb文件，反编译为一个完整的dts文件，便于阅读和理解：
	编译：
		./scripts/dtc/dtc -I dts -O dtb -o tmp.dtb  arch/arm/boot/dts/xxx.dts     //将xxx.dts  编译为 tmp.dtb
	反编译：
		./scripts/dtc/dtc -I dtb -O dts -o tmp.dts  arch/arm/boot/dts/xxx.dtb 	 //将xxx.dtb  编译为 tmp.dts

//如果要自己向设备树文件中添加结点：
1，参考内核提供的文档：Documentation/devicetree/bindings/
2, 参考同类型的单板的设备树文件
3，网上搜索
4，最后，可以通过研究内核驱动，驱动需要什么数据，就添加对应数据节点
```

###### 2.3 设备树基础语法

```
1》设备树节点语法：
一个 node 被定义成如下格式：
    [label:] node-name[@unit-address] {
        [properties definitions]
        [child nodes]
    }
“[]”表示 option可选，因此可以定义一个只有 node name 的空节点，label 方便在 dts 文件中引用

一个结点由属性和子结点组成，对于属性来说，它的值可以是：
	text string（以 null 结束），以双引号括起来，
		如：string-property = “a string”； 
	
	cells 是 32 位无符号整形数，以尖括号括起来
		如：cell-property = <0xbeef 123 0xabcd  1234>； 
		
	binary data 以方括号括起来
		如：binary-property = [0x01 0x23 0x45 0x67]；
	
	不同类型数据可以在同一个属性中存在，以逗号分格，
		如：mixed-property = “a string”, [0x01 0x23 0x45 0x67]，<0x12345678>；
	多个字符串组成的列表也使用逗号分格，
		如：string-list = “red fish”,”blue fish”;

2》设备树节点中的特殊属性
	/ {
        model = "HQYJ FS-MP1A Discovery Board";
        compatible = "st,stm32mp157a-dk1", "st,stm32mp157", "hqyj,fsmp1a";
        aliases {
            ethernet0 = &ethernet0;
            serial0 = &uart4;
            serial5 = &usart3;
        };
        chosen {
        	stdout-path = "serial0:115200n8";
        };
        ... ...
	};
	model 属性值是，它指定制造商的设备型号。推荐的格式是：“manufacturer，model”，其中 manufacturer 是一个字符串描述制造商的名称，而型号指定型号。
	
	compatible 属性值是,指定了系统的名称，是一个字符串列表，它包含了一个“<制造商>,<型号>”形式的字符串。重要的是要指定一个确切的设备，并且包括制造商的名字，以避免命名空间冲突。
	
	chosen 节点不代表一个真正的设备，但功能与在固件和操作系统间传递数据的地点一样，如根参数，取代以前 bootloader 的启动参数，控制台的输入输出参数等。
	
	port {
        #address-cells = <1>;	//表示子节点中reg属性中，使用几个u32整数来描述地址
        #size-cells = <0>;		//表示子节点中reg属性中，使用几个u32整数来描述空间大小
        ltdc_ep0_out: endpoint@0 {
            reg = <0>;
            remote-endpoint = <&sii9022_in>;
        };
	};
	例如：
	cpus {
        #address-cells = <1>;   //子节点中地址用一个u32表示
        #size-cells = <0>;     //表示子节点中用几个u32表示内存大小
        cpu0: cpu@0 {
            compatible = "arm,cortex-a7";
            device_type = "cpu";
            reg = <0>;			//0代表cpu的地址(编号),没有大小
            clocks = <&rcc CK_MPU>;
            clock-names = "cpu";
            operating-points-v2 = <&cpu0_opp_table>;
            nvmem-cells = <&part_number_otp>;
            nvmem-cell-names = "part_number";
            };
     }
     
    / {
        #address-cells = <1>;	 //子节点中地址用一个u32表示
        #size-cells = <1>;		//子节点中大小用一个u32表示
        ...
        timers2: timer@40000000 {
        	compatible = "st,stm32-timers";
        	reg = <0x40000000 0x400>;     //0x40000000表示地址，0x400表达大小
        };
    }
    
 3》状态
 	device tree 中的 status 标识了设备的状态，使用 status 可以去禁止设备或者启用设备，看下设备树规范中的 status 可选值。
 	值 			描述
	“okay” 		表示设备正在运行
	“disabled” 	表示该设备目前尚未运行，但将来可能会运行
	“fail” 		表示设备无法运行。在设备中检测到严重错误，确实如此没有修理就不可能投入运营
	“fail-sss” 	表示设备无法运行。在设备中检测到严重错误，它是没有修理就不可能投入运营。值的 sss 部分特定于设备并指示检测到的错误情况。
	
4》引用节点的方式
	第一种：phandle :  //节点中的phandle属性，它的取值必须唯一
	pic@10000000{				//中断控制器结点
		phandle = <1>;
		interrupt-controller;
	}
	device-node{
		interrupt-parent = <1>;   //通过节点的phandle属性引用节点，表示该设备的中断控制器为 pic
	};
	第二种：别名/标签
	PIC:pic@10000000{				//中断控制器结点
		interrupt-controller;
	}
	device-node{
		interrupt-parent = <&PIC>;   //通过标签引用节点，表示该设备的中断控制器为 pic，使用标签引用本质上也是使用phandle来引用，在编译dts为dtb时，编译器dtc会在dtb中插入phandle属性
	};
```

##### 3，设备树中的节点如何在驱动中被使用

###### 3.1 设备树的转换过程

```
1》第一步
*.dts ----编译 ： *.dtb文件 ----- 内核：将dtb中每一个节点转换为: struct device_node结构体形式， 设备树就被转换为一个关于device_node的一棵树
device_node这棵树的根节点: of_root

2》第二步：
	内核代码在使用设备树中的节点时，会将device_node转换为platform_device
	
在设备树中有些节点可以转换为platform_device，有些节点不能转换为platform_device
 
//根节点下含有compatible属性的子节点
	compatible属性含有特殊的值:" simple-bus", "simple-mfd","isa","arm,amba-bus"之一
	这些节点可以转换为platform_device
	
	i2c的子节点，spi的子节点等
//如何转换
	device_node中的reg属性会转化为platform_device中的内存资源
	device_node中的interrupts属性会转化为platform_device中的中断资源
	其他属性需要通过内核中的函数从device_node中获取
		struct platform_device
					|
			struct device	dev;
						|
				struct device_node	*of_node;   ---通过该指针可以获取属性中的值
				
//pdrv如何和设备树节点匹配

在pdrv中有多个匹配的接口，内核会根据顺序依次去匹配
static int platform_match(struct device *dev, struct device_driver *drv)
	
	1，比较pdev->driver_override 和 drv->name 
		return !strcmp(pdev->driver_override, drv->name);

	2，比较 dev->of_node节点中的compatible属性值和 drv->of_match_table   //设备树节点compatible属性值和of_match_table匹配
	if (of_driver_match_device(dev, drv))
	
	3, 比较pdev->name 和 pdrv->id_table         //pdev的name和id_table匹配
		return platform_match_id(pdrv->id_table, pdev) != NULL; 
	4，比较pdev->name 和 pdrv->drv.name		//pdev的name 和 pdrv父类drv中的name匹配
	return (strcmp(pdev->name, drv->name) == 0);
}

```

###### 3.2 在驱动中如何使用设备树节点

```
dtb  ----> device_node--->platform_device
设备树结点操作函数：
操作设备树的函数在下面这些头文件中有声明
peter@ubuntu:~/fs-mp157/linux/linux-stm32mp-5.4.31-r0/linux-5.4.31/include/linux$ ls of*
of_address.h  of_device.h  of_fdt.h   of_graph.h  of_iommu.h  of_mdio.h  of_pci.h  of_platform.h
of_clk.h      of_dma.h     of_gpio.h  of.h        of_irq.h    of_net.h   of_pdt.h  of_reserved_mem.h

of.h  ----操作设备树的常用函数
of_address.h -----地址相关的函数，比如：获取 reg属性中的地址，size值
of_gpio.h  ---- GPIO相关的操作函数
of_platform.h  --- 将device_node转换platform_device时用到的函数
例如： 
struct platform_device *of_device_alloc(struct device_node *np,const char *bus_id,struct device *parent);
struct platform_device *of_find_device_by_node(struct device_node *np);

1》找结点
	//根据路径找结点，比如： "/"根节点， "/memory" -- 对应的是memory节点
	static inline struct device_node *of_find_node_by_path(const char *path)

	//根据名称找结点，节点中要定义name属性
	struct device_node *of_find_node_by_name(struct device_node *from,const char *name);
	
	//根据节点类型找结点,节点中定义了device_type属性
	struct device_node *of_find_node_by_type(struct device_node *from,	const char *type);
	
	//根据compatible属性找
	struct device_node *of_find_compatible_node(struct device_node *from,const char *type, char *compat);
	
	//根据节点中的phandle属性找
	struct device_node *of_find_node_by_phandle(phandle handle);
	
2》找结点中的属性
	1)获取属性的结构体指针
	struct property *of_find_property(const struct device_node *np, const char *name, int *lenp);
	//返回属性结构体指针：struct property *
	2)获取属性的值
	static  void *of_get_property(const struct device_node *node,const char *name,	int *lenp)
	//返回属性值
	3)获取属性值的元素个数
	int of_property_count_elems_of_size(const struct device_node *np,const char *propname, int elem_size);
	//根据名字找到节点的属性，确定属性的值有多少个元素
	在设备树中，节点应该是：
	xxx_node{
		xxx_pp_name = <0x54004000 0x400>  <0x5000C000 0x400>;
	};
	of_property_count_elems_of_size(np,"xxx_pp_name",8);   //返回的个数为：2
	of_property_count_elems_of_size(np,"xxx_pp_name",4);   //返回的个数为：4
	
	4)读取属性的整型值u32/u64
	int of_property_read_u32(const struct device_node *np, const char *propname,u32 *out_value)
	int of_property_read_u64(const struct device_node *np,const char *propname, u64 *out_value);
	例如： 
	xxx_node{
		name1 = <0x54004000>;
		name2 = <0x54004000 0x00004000>;
	};
	of_property_read_u32(np,"name1",&val) , val1的值为：0x54004000
	of_property_read_u64(np,"name2",&val2), val2的值为：0x00004000 54004000
	
	5）获取某个u32的值
	int of_property_read_u32_index(const struct device_node *np,char *propname, u32 index, u32 *out_value);
	int of_property_read_u64_index(const struct device_node *np,char *propname, u32 index, u64 *out_value);
	例如：
	xxx_node{
		name = <0x54004000 0x5400c000>;
	};
	of_property_read_u32_index(np,"name",1,&val);   val值为：0x5400c000
	
	6)读取数组的值
	int of_property_read_variable_u8_array(const struct device_node *np,
					char *propname, u8 *out_values,size_t sz_min, size_t sz_max);
	int of_property_read_variable_u16_array(const struct device_node *np,
					const char *propname, u16 *out_values,size_t sz_min, size_t sz_max);
	int of_property_read_variable_u32_array(const struct device_node *np,
					const char *propname,u32 *out_values,size_t sz_min,size_t sz_max);
	int of_property_read_variable_u64_array(const struct device_node *np,
					const char *propname,u64 *out_values,size_t sz_min,size_t sz_max);
	例如：
	xxx_node{
		name = <0x54004000 0x5400c000>;
	};
	char val[10];
	of_property_read_variable_u8_array(np,"name",val,1,10); 
	val中的值为：{0x00,0x40,0x00,0x54,0x00,0xc0,0x00,0x54}
	of_property_read_variable_u16_array(np,"name",val,1,10);
	val中的值为：{0x4000,0x5400,0xc000,0x5400};
	
	7)读字符串
	  int of_property_read_string(const struct device_node *np,char *propname,const char **out_string);
	  
	9）获取字符串数组
		#define of_property_for_each_string(np, propname, prop, s)	\
            for (prop = of_find_property(np, propname, NULL),	\
                s = of_prop_next_string(prop, NULL);		\
                s;						\
                s = of_prop_next_string(prop, s))
  		例如：
        of_property_for_each_string(np, "string_array", prop, str)    //循环获取属性的多个字符串值
                printk("str = %s\n",str);
       
    10）获取32位整形数组
    	#define of_property_for_each_u32(np, propname, prop, p, u)	\
            for (prop = of_find_property(np, propname, NULL),	\
                p = of_prop_next_u32(prop, NULL, &u);		\
                p;						\
                p = of_prop_next_u32(prop, p, &u))
        例如： 
         of_property_for_each_u32(np, "ages", prop, p, val)  //循环获取32位整数数组的元素
                printk("val = %d\n",val);
	  
例如：在设备树文件中创建一个节点，如下：

	1》在设备树文件 arch/arm/boot/dts/stm32mp157a-fsmp1a.dts  添加节点
	/{
		farsight:stm32mp157_test_dev@54004000{
            compatible = "stm32mp157,test_dev";
            reg = <0x54004000 0x400 0x5400c000 0x1000>;
            string_array = "jack","rose";
            ages = <23 21>;
            bin = [080b0507];
            hello;
            sun{
                sun_name = "robin";
                age = <12>;
            };
        };
	};
	&farsight{
        ages = <32 27>;
    };
	2》编译设备树文件，并更新
        make ARCH=arm -j4  stm32mp157a-fsmp1a.dtb LOADADDR=0xC2000040
        cp arch/arm/boot/dts/stm32mp157a-fsmp1a.dtb /tftpboot/
3》编写驱动代码 ---实现内核中的pdrv模块
		int test_devtree_pdrv_probe(struct platform_device * pdev)
{
	struct resource *res1,*res2;
	const struct device_node *np = pdev->dev.of_node;
	int len,*p,val,age;
	char *str,*name;
	struct property *prop;
	u8 b[4];
	struct device_node * sub_node;
	printk("---------^_^ %s--------------\n",__FUNCTION__);

    //获取平台资源
	res1  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	res2 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	printk("res1->start = %x\n",res1->start);
	printk("res2->start = %x\n",res2->start);

	//获取设备数节点中的自定义属性
	str = (char*)of_get_property(np, "string_array", &len);     //获取属性的一个字符串值
	printk("str = %s,len = %d\n",str,len);

	//prop_str = of_find_property(np, "string_array", &len);
	of_property_for_each_string(np, "string_array", prop, str)    //循环获取属性的多个字符串值
		printk("str = %s\n",str);

	of_property_for_each_u32(np, "ages", prop, p, val)  //循环获取32位整数数组的元素
		printk("val = %d\n",val);

	of_property_read_u8_array(np, "bin", b, 4);
	printk("bin = %x %x %x %x\n",b[0],b[1],b[2],b[3]);

	//获取子节点
	sub_node = of_find_node_by_name(np, "sun");
	if(sub_node){
		of_property_read_string(sub_node, "sun_name", (const char * *)&name);
		of_property_read_u32(sub_node, "age", (u32 *)&age);
		printk("name = %s,age = %d\n",name,age);
	}
	return 0;

}

```

#### 三，使用设备树实现led驱动

##### 1 ,在设备树文件中添加led的信息

```
在设备树文件arch/arm/boot/dts/stm32mp157a-fsmp1a.dts中添加结点：

led_test1@0x54004000{
        compatible = "stm32mp157,led_test1";
        reg = <0x54004000 0x400>;
        dev-name = "led02";               //设备结点名称
        mode_clear =   <0x3f>;    //0x3f
        mode_data  =   <0x15>;   //0x15
        shift      =   <5>;      //5
        odr        =   <0x7>;            //0x7
        minor      =   <8>;
    };

//编译，并更新设备树文件
make ARCH=arm -j4  stm32mp157a-fsmp1a.dtb LOADADDR=0xC2000040
cp arch/arm/boot/dts/stm32mp157a-fsmp1a.dtb /tftpboot/
```

##### 2，编写平台总线的pdrv层代码

```
1》创建匹配table
	const struct of_device_id	led_of_match_table[] = {
        {.compatible = "stm32mp157,led_test1"},
    };


    //1,实例化pdrv对象
    struct platform_driver  led_pdrv = {
        .probe 	 	=  led_pdrv_probe,
        .remove		=  led_pdrv_remove,
        .driver 	=	{
            .name 	=	"stm32mp157_led",    //必须要赋值：ls /sys/bus/platform/drivers/stm32mp157_led/
            .of_match_table = led_of_match_table,
        },
    };
2》在probe中获取设备树结点属性信息
	int led_pdrv_probe(struct platform_device *pdev)
    {
        int ret;

        struct resource *res1;
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
        of_property_read_u32(led_dev->np, "mode_data", &led_dev->mode_data);
        of_property_read_u32(led_dev->np, "mode_clear", &led_dev->mode_clear);
        of_property_read_u32(led_dev->np, "odr", &led_dev->odr);
        of_property_read_u32(led_dev->np, "shift", &led_dev->shift);

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

        //获取平台资源
        res1 = platform_get_resource(pdev, IORESOURCE_MEM, 0); //资源编号从0开始，表示同类型资源编号
        if(!res1){
            printk("platform_get_resource error\n");
            goto err_misc_deregister;
        }
        printk("res1->start = %x\n",res1->start);


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
 3》在接口中使用获取的属性信息
 	int led_drv_open(struct inode *inode, struct file *filp)
    {
        printk("-----------^_^ %s-------------\n",__FUNCTION__);
        //将gpio设置为输出模式
        led_dev->gpioz->MODER &=  ~(led_dev->mode_clear << led_dev->shift *2);
        led_dev->gpioz->MODER |= led_dev->mode_data << led_dev->shift *2;

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
            led_dev->gpioz->ODR |= led_dev->odr << led_dev->shift;
        }else{
            //关灯
            led_dev->gpioz->ODR &= ~(led_dev->odr  << led_dev->shift);
        }

        return size;
    }
```

