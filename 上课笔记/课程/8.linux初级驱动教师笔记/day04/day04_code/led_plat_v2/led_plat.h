
#ifndef  __led_plat_H__
#define  __led_plat_H__

//定义平台自定义数据类型
struct led_platdata{
	char *name;					 //设备结点名称
	unsigned int mode_clear;     //0x3f
	unsigned int mode_data;     //0x15
	int shift ;					//5
	unsigned int odr;           //0x7
	int minor;                  //次设备号
};


#endif  



