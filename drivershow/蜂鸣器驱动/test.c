#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<sys/ioctl.h>
int main(void)
{
	int fpd;
	int retur=0,i=0;
	char buffer[200]="hello,everyone\n";
	fpd=open("/dev/hello",O_CREAT|O_RDWR,0X666);
#if 0
	retur=write(fpd,buffer,strlen(buffer)+1);	
	printf("user write retur= %d\n",retur);
	memset(buffer,'\0',sizeof(buffer));
	retur= read(fpd,buffer,200);
	printf("user read retur= %d\n",retur);
	printf("user :  %s\n",buffer);
#endif
	while(i<5)
	{
		ioctl(fpd,1,0);
		sleep(1);
		ioctl(fpd,0,0);
		sleep(1);
		i++;
	}
	close(fpd);
	return ;
}
