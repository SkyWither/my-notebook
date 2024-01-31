#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


//定义控制led命令
#define LED_ALL_ON     _IO('L', 0xa)
#define LED_ALL_OFF	   _IO('L', 0xb)
#define LED_NUM_ON     _IOW('L', 0xc, int)
#define LED_NUM_OFF    _IOW('L', 0xd, int)


int main(int argc,char** argv)
{
	int fd;
	int i;

	if(argc != 2){
		fprintf(stderr,"Usage: %s </dev/xxx>\n",argv[0]);
		exit(1);
	}

	//打开设备
	fd = open(argv[1],O_RDWR);
	if(fd < 0){
		perror("open");
		exit(1);
	}
	for(i = 0; i <7; i++){
		ioctl(fd,LED_NUM_ON,i%3+1);
		usleep(500000);
		ioctl(fd,LED_NUM_OFF,i%3+1);
		usleep(500000);
	}

	ioctl(fd,LED_ALL_ON);
	sleep(3);

	close(fd);

	return 0;
}
