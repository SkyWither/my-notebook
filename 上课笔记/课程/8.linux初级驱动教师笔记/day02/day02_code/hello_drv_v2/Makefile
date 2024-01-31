#指定内核源码的位置
KERN_DIR = /home/farsight/mp157/kernel/linux-5.4.31
CUR_DIR = `pwd`
#CUR_DIR =  $(shell pwd)

MODULE_NAME1 = hello_drv
MODULE_NAME2 = fun

all:
		make -C $(KERN_DIR)  M=$(CUR_DIR) modules

clean:	
		make -C $(KERN_DIR)  M=$(CUR_DIR) clean

install:
		cp *.ko  /opt/rootfs/drv_modules

#指定要编译哪个文件
obj-m = $(MODULE_NAME1).o
obj-m += $(MODULE_NAME2).o

