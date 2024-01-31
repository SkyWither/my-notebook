#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

//定义按钮数据类型
struct mp157_key_data{
	int code;
	int value;     //1 ---按下 ，0---松开
};


int main(int argc,char** argv)
{
	int fd,fd_led;
	int i,on;
	struct mp157_key_data kdata;

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

	fd_led = open("/dev/led03",O_RDWR);
	if(fd_led < 0){
		perror("open");
		exit(1);
	}
	//读取按键信息
	while(1){
		bzero(&kdata,sizeof(kdata));
		if(read(fd,&kdata,sizeof(kdata)) < 0){
			perror("read");
			//exit(1);
		}
		switch(kdata.code){
			case KEY_1:
				if(kdata.value){
					printf("按下--key1\n");
				}else{
					printf("松开--key1\n");
				}
				on = 1;
				write(fd_led,&on,sizeof(on));
				break;
			case KEY_2:
				if(kdata.value){
					printf("按下--key2\n");
				}else{
					printf("松开--key2\n");
				}
				on = 0;
				write(fd_led,&on,sizeof(on));
				break;
			case KEY_3:
				if(kdata.value){
					printf("按下--key3\n");
				}else{
					printf("松开--key3\n");
				}
				break;
		}
	}
	
	close(fd);

	return 0;
}
