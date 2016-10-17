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




unsigned char control_state=0x40;
int flag_timer=0;

unsigned char *byte_stuffing(unsigned char *str,int *size)
{
int i;
int j;
int k;
int *ponto_crit=(int *) malloc(50);
int hit=0;
unsigned char *buffer=(unsigned char*) malloc((*size));
for(i=0;i<((*size));i++)	
{
	if((i>0) && (i<((*size)-1) ))
	{
		if( (str[i]==0x7e) || (str[i]==0x7d) )
		{
			ponto_crit[hit]=i;	
			hit++;
			//printf("\nindice critico: %d\n",i);
		}
	}
	memcpy(&buffer[i],&str[i],1);	
	//printf("\nNO byte buffing PRE PROCESSAMENTO buffer Indice %d: %#08x----%c\n",i,buffer[i],buffer[i]); 	
}
str=(unsigned char*) realloc(str,(*size)+hit);
/*for(i=0;i<hit;i++)
printf("\nPontos criticos: %d\n",ponto_crit[i]);*/
//sleep(5);
i=1;
j=0;
k=1;
while(i<((*size)))
{
	if(ponto_crit[j]==i)
	{
		str[k]=0x07d;	
		str[k+1]=(buffer[i]^0x20);
	//	printf("\nIndice i: %d -- Indice k: %d -- str[k]: %#08x -- str[k+1]: %#08x\n",i,k,str[k],str[k+1]); 		
		i++;
		j++;
		k=k+2;
	//	sleep(4);
	}
	else 
	{
		memcpy(&str[k],&buffer[i],1);
		i++;	
		k++;
	}
	//printf("\nNO byte buffing str Indice %d: %#08x----%c\n",i,str[i],str[i]);
}
//sleep(10);
(*size)=(*size)+hit;
/*for(i=0;i<(*size);i++)
printf("\nNO byte buffing str Indice %d: %#08x----%c\n",i,str[i],str[i]);*/
//sleep(10);
free(buffer);
free(ponto_crit);
return str;
}

void xor_func(unsigned char *str,int size,int c,int pack_size)
{
int i;
int indice2=size-2;
	//printf("\n size: %d\n",size);
	//printf("\n indice: %d\n",indice2);
	//printf("\n pack size: %d\n",pack_size);
	//sleep(2);
	if(c==1)//c=1 estamos a tratar o BCC1
	{
		str[3]=str[1]^str[2];
		//str[3]=0;
		
	}	
	else if(c==2)
	{
		str[indice2]=str[(size-2-pack_size)]^str[(size-1-pack_size)];  //isto não bom o suficiente ... Pode ter mais que um L(L1 e L2)...
		for(i=(size-pack_size);i<indice2;i++)
		{
			str[indice2]=str[i]^str[indice2];
		}    
		//str[indice2]=0;	
	}

//printf("\n SAI");
}

void alm_func()
{
	//fprintf(stderr,"\nALARM");
	flag_timer=2;
}

int timeout_func(int *count_timer,unsigned char control,int length, int fd,int *count_bcc)
{

int retornar=FALSE;
unsigned char *buffer = (unsigned char*) malloc(length);
int i;
unsigned char testa;
int l=0;
	
	
	//printf("\nAtiva alarme\n");
	alarm(3);
	while ( (flag_timer==0) && (retornar==0) ) 
	{
        	l=read(fd,&(buffer[0]),1);
		if(l==-1)
		{
			perror("Error printed by perror");					
		}
       		if (buffer[0] == 0x7E) 
		{
		    	//printf("Encontrou a flag inicial, vou ler\n");
			i=1;
			while( (i<length) && (flag_timer==0) ) 
			{
                		l=read(fd,&(buffer[i]), 1);
				if(l>0)
				{
					//printf("\n Indice %d: %#02x\n",i,buffer[i]);
					i++;								
				}		
			
			}
			//printf("\n Flag timer: %d\n",flag_timer);
			if(flag_timer==0) //nesta altura temos uma frame recebida e o alarm ainda não foi chamado
			{		
					alarm(0);
				//	printf("\n Alarme off\n");
					testa=( ( (control_state << 1) ^ 0x80 ) | 0x03 );
					//printf("\n testa : %#02x\n",testa);
					//printf("\n Ack: %#02x\n",buffer[2]);		
					if((testa^buffer[2])==0)	
					{
						//printf("\n testa : %#08x\n",testa);	
						//printf("\n ENTROU NO RETORNAR TRUE\n");
						retornar=true;
					}
					//retornar=1;
					else	
					{
						(*count_bcc)++;	
						printf("\n REJ numero: %d\n",*count_bcc);
						//sleep(2);
						flag_timer=1;
						
					}
						
			
					
			}		
		}
	}
if(flag_timer==2)//flag timer =1 se foi bcc =2 se foi timer
(*count_timer)++;
//fprintf(stderr, "\nSaiu timout func");
flag_timer=0;

free(buffer);
return retornar;
}

int llwrite(int fd,unsigned char *buffer,int length)
{
//struct termios oldtio,newtio;
unsigned char frame_header[4];
unsigned char frame_trailer_cmd;
unsigned char frame_trailer[2];
unsigned char *full_frame;
int confirm=FALSE;
int retorna;
int contador_char=0;
int i;
int size;
int contador_timer=0;
int contador_bcc=0;
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
//while(confirm==FALSE)		
//{	
			
		if(buffer[0]==0x02)//ini
		{
			//printf("\n INIC \n");
			//sleep(2);
			frame_header[0]=0x7E;
			frame_header[1]=0x03;
			frame_header[2]=control_state;
			frame_header[3]=0x00;
			frame_trailer_cmd=0x7E;
			size=length+sizeof(frame_header)+sizeof(frame_trailer_cmd);
			
			full_frame=(unsigned char*) malloc(size);

			memcpy(full_frame,frame_header,sizeof(frame_header));
		
			memcpy(&full_frame[sizeof(frame_header)],buffer,length); 
			memcpy(&full_frame[size-sizeof(frame_trailer_cmd)],&frame_trailer_cmd,sizeof(frame_trailer_cmd));
			/*for(i=0;i<size;i++)
			{			
				printf("\n Indice %d: %#08x\n",i,full_frame[i]);
			}	*/		
			//sleep(10);
			xor_func(full_frame,size,1,length);
			//printf("Sai xor");
			/*for(i=0;i<size+1;i++)
			{			
				printf("\n Indice %d: %#08x\n",i,full_frame[i]);
			}	*/	
		}
		/*else if(buffer[1]==0x03)//end
		{

		}*/
		else if(buffer[0]==0x01)//dados
		{	
			//printf("\n DADOS \n");	
			//sleep(10);
			frame_header[0]=0x7E;
			frame_header[1]=0x03;
			frame_header[2]=control_state;
			frame_header[3]=0x00;
			frame_trailer[0]=0x00;	
			frame_trailer[1]=0x7E;
			size=length+sizeof(frame_header)+sizeof(frame_trailer);
			full_frame=(unsigned char*) malloc(size);

			memcpy(full_frame,frame_header,sizeof(frame_header));
		
			memcpy(&full_frame[sizeof(frame_header)],buffer,length); 
			memcpy(&full_frame[size-sizeof(frame_trailer)],frame_trailer,sizeof(frame_trailer));

			xor_func(full_frame,size,1,(buffer[2]*256+buffer[3]));
			xor_func(full_frame,size,2,(buffer[2]*256+buffer[3]));
			/*for(i=0;i<size;i++)
			{			
				printf("\n Indice PRE BYTE STUFF %d: %#02x----%c\n",i,full_frame[i],full_frame[i]);
			}*/
			//printf("\nTamanho pré bit stuffing: %d\n",size);
			full_frame=byte_stuffing(full_frame,&size);
			/*printf("\nTamanho pós bit stuffing: %d\n",size);*/			
			//sleep(10);
//			printf("Sai xor");
			
		/*	for(i=0;i<size;i++)
			{			
				printf("\n Indice %d: %#08x\n",i,full_frame[i]);
			}*/
				
	
		}

		while( 1 )
		{		
			contador_char=0;	
			for(i=0;i<(size);i++)
			{		
					l=write(fd,&(full_frame[i]),1);
					//printf("\n Indice %d: %#02x----%c\n",i,full_frame[i],full_frame[i]);
					if(l==-1)
					{
						perror("Error printed by perror");
						sleep(6);					
					}
					else if(l>0)
					{										
						contador_char++;
						//printf("\n Indice %d: %#02x----%c----contador: %d\n",i,full_frame[i],full_frame[i],contador_char);
						
					}
					
			}
			//sleep(10);
			//printf("\ncontrol_state: %#08x\n",control_state);
						
			confirm=timeout_func(&contador_timer,control_state,5,fd,&contador_bcc);
			//printf("CONFIRM: %d",confirm);
			//sleep(2);	
			//confirm=1;
			if(confirm==1)//tudo tope
			{
				//printf("\n Tudo tope\n");
				control_state=(control_state ^ 0x40);
				//sleep(3);
				retorna=contador_char;		
				break;
			
			}
			else if(contador_timer==3)
			{
				printf("\nTIMEOUT\n");
				retorna=-1;
				break;
		 		}	
			else if(contador_bcc==3)
			{
				printf("\nRecebidos 3 REJs consecutivos\n");	
				for(i=0;i<size;i++)
				{			
					printf("\n Indice %d: %#08x\n",i,full_frame[i]);
				}
				sleep(10);
				retorna=-1;				
				break;
			} 	

		}
		
//}		

free(full_frame);
//printf("\n vou retornar: %d\n",retorna);
//sleep(3);
return retorna;
}

