#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <poll.h>

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
	struct pollfd fds[2];
	char buf[100];

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

	//使用多路复用同时监测标准输入和key
	fds[0].fd = 0;       //标准输入
	fds[0].events = POLLIN;


	fds[1].fd = fd;      //按键
	fds[1].events = POLLIN;

	//读取按键信息
	while(1){
		if(poll(fds,2,-1) < 0){   //同时监测标准输入和按键
			perror("poll");
			exit(1);
		}

		//如果标准输入可读，则读数据
		if(fds[0].revents & POLLIN){
			fgets(buf,sizeof(buf),stdin);
			printf("%s",buf);
		}

		//如果按键可读，则读按键信息
		if(fds[1].revents & POLLIN){
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
		}
	}

	close(fd);

	return 0;
}
