#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PAGE_SIZE 4096
#define IOC_GET_DATA _IOR('K', 0x123, char*)

//定义按钮数据类型
struct mp157_key_data{
	int code;
	int value;     //1 ---按下 ，0---松开
};


int main(int argc,char** argv)
{
	int fd;
	int i,on;
	struct mp157_key_data kdata;
	char *buf,str[PAGE_SIZE];

	if(argc != 2){
		fprintf(stderr,"Usage: %s </dev/xxx>\n",argv[0]);
		exit(1);
	}

	//打开设备
	fd = open(argv[1],O_RDWR);
	//fd = open(argv[1],O_RDWR | O_NONBLOCK);
	if(fd < 0){
		perror("open");
	//	exit(1);
	}

	//1,映射一块物理内存到进程的空间中
	buf = (char*)mmap(NULL, PAGE_SIZE, PROT_READ| PROT_WRITE,MAP_SHARED,fd, 0);
	if(buf < 0){
		perror("mmap");
		exit(1);
	}

	//2，向空间中写一个字符串
	strcpy(buf,"hello world");
	
	sleep(1);

	//3,测试是否能够获取字符串
	if(ioctl(fd,IOC_GET_DATA,str) < 0){
		perror("ioctl");
		exit(1);
	}		
	printf("str = %s\n",str);

	//读取按键信息
	while(1){
		bzero(&kdata,sizeof(kdata));
		if(read(fd,&kdata,sizeof(kdata)) < 0){
			perror("read");
			//exit(1);
		}
		if(kdata.code == KEY_2){
			if(kdata.value){
				printf("按下--中键\n");
			}else{
				printf("松开--中键\n");
			}
		}
		sleep(1);
	}
	
	close(fd);

	return 0;
}
