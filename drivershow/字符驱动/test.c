#include<stdio.h>
#include<fcntl.h>
int main(void)
{
	int fpd;
	fpd=open("/dev/hello",O_CREAT,0X666);
	close(fpd);
	return ;
}
