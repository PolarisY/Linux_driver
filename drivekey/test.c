#include<stdio.h>
#include<fcntl.h>			//fcntl.h定义了很多宏和open,fcntl函数原型
							//unistd.h定义了更多的函数原型
							//close（关闭文件）
#include<stdlib.h>
int main(void)
{
	fd_set rdfs;
	int ret;
	int key_value[4];
	int i=0;
	int fd=open("/dev/key",O_CREAT|O_NONBLOCK,0x666);
	if(fd<0)
	{
		printf("failed in openning /dev/key\n");
		return 0;
	}
	while(1)
	{
		FD_ZERO(&rdfs);
		FD_SET(fd,&rdfs);
		ret=select(fd+1,&rdfs,NULL,NULL,NULL);
		if(ret<0)
		{
			printf("call select function failed\n");
			exit(0);
		}
		else if(ret==0)
		{
			printf("select time out\n");
		}
		else if(ret>0)
		{
			if(FD_ISSET(fd,&rdfs))
			{
				ret=read(fd,key_value,sizeof(key_value));
				if(ret!=sizeof(key_value)/sizeof(int))
				{
					printf("user space read failed\n");
					continue;
				}
				else
				{
					for(i=0;i<4;i++)
					{
						if(key_value[i]==0)
						{
							printf("key[%d] pressed\n",i);
						}
					}			
				}
			}
		}
	}
	close(fd);
	return 1;
}
