#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



int main(void)
{
	int fd;
	int i,on;

	//打开设备
	fd = open("/dev/led2",O_RDWR);
	if(fd < 0){
		perror("open");
		exit(1);
	}
#if 0
	for(i = 0; i <7; i++){
		on = 1;
		write(fd,&on,sizeof(on));
		usleep(500000);
		on = 0;
		write(fd,&on,sizeof(on));
		usleep(500000);
	}
#else
	while(1){
		printf("请选择:1 --开灯，0--关灯\n");
		scanf("%d",&on);
		write(fd,&on,sizeof(on));
	}
#endif
	
	close(fd);

	return 0;
}
