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
#include <errno.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1





bool flag_timer=FALSE;

void xor_func(char *str,int size,int c,int pack_size)
{
int i;
int indice2=size-2;
	if(c==1)//c=1 estamos a tratar o BCC1
		str[3]=str[1]^str[2];
	else if(c==2)
	{
		str[indice2]=str[(size-2-pack_size)]^str[(size-1-pack_size)];  //isto não bom o suficiente ... Pode ter mais que um L(L1 e L2)...
		for(i=(size-pack_size);i<indice2;i++)
		{
			str[indice2]=str[i]^str[indice2];
		}	
	}
printf("\n SAI");
}

void alm_func()
{
	fprintf(stderr,"\nALARM");
	flag_timer=TRUE;
}

int timeout_func(int *count,int control,int length, int fd)
{

int retornar=FALSE;
char *buffer = (char*) malloc(length);
int i;
int l;
	
	alarm(3);
	printf("Ativa alarme");
	while ( (flag_timer==0) && (retornar==0) ) 
	{
		
        	l=read(fd,&(buffer[0]),1);
		if(l==-1)
		{
			perror("Error printed by perror");
			sleep(6);					
		}
       		if (buffer[0] == 0x7E) 
		{
		    	printf("Encontrou a flag inicial, vou ler\n");
		    	for(i=0;i<length;i++) 
			{
                		(void) read(fd,&(buffer[i]), 1);
			}
			if(flag_timer==FALSE)//nesta altura temos uma frame recebida e o alarm ainda não foi chamado
				{
					alarm(0);
					if(control==buffer[2])
					retornar=TRUE;
					
					
				}		
		}
	}
fprintf(stderr, "\nSaiu timout func");
flag_timer=0;
(*count)++;
free(buffer);
return retornar;
}

int llwrite(int fd,char *buffer,int length)
{
//struct termios oldtio,newtio;
char control_state=0x40;
char frame_header[4];
char frame_trailer[2];
char *full_frame;
int confirm=FALSE;
int i;
int size;
int contador=0;
int l;
/*int

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

   set input mode (non-canonical, no echo,...) 
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;    inter-character timer unused 
newtio.c_cc[VMIN]     = 0;*/ 

setbuf(stdout, NULL);
(void) signal(SIGALRM, alm_func);
while(confirm==FALSE)		
{	
			
		if(buffer[0]==0x02)//ini
		{
			frame_header[0]=0x7E;
			frame_header[1]=0x03;
			frame_header[2]=control_state;
			frame_header[3]=0x00;
			frame_trailer[1]=0x7E;
			size=length+sizeof(frame_header)+sizeof(frame_trailer);
			
			full_frame=(char*) malloc(size);

			memcpy(full_frame,frame_header,sizeof(frame_header));
		
			memcpy(&full_frame[sizeof(frame_header)],buffer,length); 
			memcpy(&full_frame[size-sizeof(frame_trailer)],frame_trailer,sizeof(frame_trailer));
			for(i=0;i<size;i++)
			{			
				printf("\n Indice %d: %#08x\n",i,full_frame[i]);
			}			
			xor_func(full_frame,size,1,length);
			printf("Sai xor");
			for(i=0;i<size;i++)
			{			
				printf("\n Indice %d: %#08x\n",i,full_frame[i]);
			}		
		}
		/*else if(buffer[1]==0x03)//end
		{

		}*/
		/*else if(buffer[1]==0x01)//dados
		{

		}*/
		
		while( (contador<3) && (confirm==FALSE) )
		{		
				printf("\nreescrever....\n");
				for(i=0;i<(size);i++)
				{		
					l=write(fd,&(full_frame[i]),1);
					if(l==-1)
					{
						perror("Error printed by perror");
						sleep(6);					
					}
					
				}
				control_state=control_state ^ 0x40;
				confirm=timeout_func(&contador,control_state,size,fd);
				printf("contador: %d",contador);
				sleep(2);	
		}
		if( (confirm==0) && (contador<3) )//tem que se reenviar a mensagem
		{
			printf("\nTimeout.\nA reenviar a mensagem...");
			
		}
		else if(contador==3)
		{
			printf("\nTIMEOUT\n");
			confirm=1;
		}
}		

free(full_frame);

return 0;
}

