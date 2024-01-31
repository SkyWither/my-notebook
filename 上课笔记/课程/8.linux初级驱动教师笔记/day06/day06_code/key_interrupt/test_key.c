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
	int fd;
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
