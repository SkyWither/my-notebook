#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



int main(int argc,char** argv)
{
	int fd;
	int i,on;

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
		on = 1;
		write(fd,&on,sizeof(on));
		usleep(500000);
		on = 0;
		write(fd,&on,sizeof(on));
		usleep(500000);
	}
	
	close(fd);

	return 0;
}
