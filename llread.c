#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1



int llread(int fd,char *buffer)
{
bool final_recep=FALSE;
int i;
int j;
	while (final_recep==FALSE) 
	{
        read(fd,&(buffer[0]),1);
		if (buffer[0] == 0x7E) 
		{
			printf("\nReceptor encontrou a flag inicial, vou ler\n");
			for(i=1;( (buffer[i-1]!=0x7E) || (i!=1) );i++) 
			{
				printf("\nyooo\n");
			        (void)read(fd,&(buffer[i]),1);
			        printf("\n Indice %d:  %#08x\n",i,buffer[i]);
				sleep(2);
						
			}
			/*buffer[3]=buffer[3]^0x40;
			for(j=0;j<(i+1);j++)
			{
				write(fd,&(buffer[i]),1);
			}*/	
		}	
	}
return 0;
}
