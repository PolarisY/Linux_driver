#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<sys/ioctl.h>/*ioctl()是I/O操作的杂货箱，很多事情都要依靠它来完成*/
int main(void)
{
	int fpd;
	int retur=0,i=0;
	char buffer[200]="hello,everyone\n";
	fpd=open("/dev/hello",O_CREAT|O_RDWR,0X666);
#if 0
	/向设备发送数据，成功时该函数返回写入的字节数。如果此函数未被实现，当用户进行
	write()系统调用时，将得到-EINVAL 返回值。*/
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
