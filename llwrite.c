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
#include "headers.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int rej_count=0;
//int i_total=0;
int i_recep=0;
//int i_envios=0;


unsigned char control_state=0x40;
int flag_timer=0;
//int counte=0;
unsigned char *byte_stuffing(unsigned char *str,int *size)
{
	int i;
	int j;
	int k;
	int *ponto_crit=(int *) malloc(*size);
	int hit=0;
	unsigned char *buffer=(unsigned char*) calloc((*size),sizeof(unsigned char));

	for(i=0;i<((*size));i++)	{
		if((i>0) && (i<((*size)-1) )){
			if( (str[i]==0x7e) || (str[i]==0x7d) ){
				ponto_crit[hit]=i;	
				hit++;			
			}
		}
		memcpy(&buffer[i],&str[i],1);	
	}

	str=(unsigned char*) realloc(str,(*size)+hit);

	i=1;
	j=0;
	k=1;

	while(i<((*size))){
		if( (ponto_crit[j]==i) && (j<hit) ){
			str[k]=0x07d;	
			str[k+1]=(buffer[i]^0x20);
			i++;
			j++;
			k=k+2;
		}
		else {
			memcpy(&str[k],&buffer[i],1);
			i++;	
			k++;
		}
	}	
	(*size)=k;
	free(buffer);
	free(ponto_crit);
	return str;
}

void xor_func(unsigned char *str,int size,int c,int pack_size)
{
	int i;
	int indice2=size-2;

	if(c==1)//c=1 estamos a tratar o BCC1
	{
		str[3]=str[1]^str[2];
	}	
	else if(c==2)
	{
		str[indice2]=str[(size-2-pack_size)]^str[(size-1-pack_size)];  
		for(i=(size-pack_size);i<indice2;i++)
		{
			str[indice2]=str[i]^str[indice2];
		}  
	}
}
void alm_func(){
	//fprintf(stderr,"\nTimeout\n");
	flag_timer=2;
}

int timeout_func(int *count_timer,unsigned char control,int length, int fd,int *count_bcc,int *rej_cnt,int *rec_cnt,int time_out){
	int retornar=FALSE;
	unsigned char *buffer = (unsigned char*) malloc(length);
	int i;
	unsigned char testa;
	int l=0;
	
	alarm(time_out);
	while ( (flag_timer==0) && (retornar==0) ) {
        l=read(fd,&(buffer[0]),1);
		if(l==-1){
			perror("Error printed by perror");					
		}
       	if ( (buffer[0] == 0x7E) && (l>0) ) {
			i=1;
			while( (i<length) && (flag_timer==0) ) {
                		l=read(fd,&(buffer[i]), 1);
				if(l>0){				
					i++;								
				}				
			}
		
		
			if(flag_timer==0){ //nesta altura temos uma frame recebida e o alarm ainda não foi chamado
					
				(*rec_cnt)++;
				alarm(0);
				testa=( ( (control_state << 1) ^ 0x80 ));		
				if(( (testa | 0x05) ^ buffer[2] )==0)	{
										
					retornar=true;					
				}
				//retornar=1;
				else if(( (testa | 0x01) ^ buffer[2] )==0) {
					(*count_bcc)++;	
					(*rej_cnt)++;
					
					//printf("\n REJ numero: %d\n",*count_bcc);
					tcflush(fd, TCIOFLUSH);

					flag_timer=1;					
				}
				else  
				{
					flag_timer=1;
					//printf("\nAlgum tipo de erro Ack: %#02x\n",buffer[2]);
					retornar=true;
				}				
			}		
		}
	}
	if(flag_timer==2)//flag timer =1 se foi bcc =2 se foi timer
	(*count_timer)++;
	flag_timer=0;

	free(buffer);
	return retornar;
}

int llwrite(int fd,unsigned char *buffer,int length,infoo *sct_info)
{

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


	setbuf(stdout, NULL);
	(void) signal(SIGALRM, alm_func);
		
	if(buffer[0]==0x02 || buffer[0]==0x03){//ini
				
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

		xor_func(full_frame,size,1,length);
		full_frame=byte_stuffing(full_frame,&size);
		(sct_info->i_sent)++;
		//printf("Sai xor");

	}

	else if(buffer[0]==0x01){//dados

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
		xor_func(full_frame,size,2,(length));
		full_frame=byte_stuffing(full_frame,&size);

		(sct_info->i_sent)++;
	}

	while(1) {		
		contador_char=0;	
		for(i=0;i<(size);i++){		
				l=write(fd,&(full_frame[i]),1);
			
				if(l==-1){
					perror("Error printed by perror");
					sleep(6);					
				}
				else if(l>0){										
					contador_char++;				
				}			
		}
		confirm=timeout_func(&contador_timer,control_state,5,fd,&contador_bcc,&(sct_info->rej_count),&(sct_info->i_received),sct_info->time_out);
		
		//printf("\n contador_timer: %d\n",contador_timer);
		if(confirm==1){	
			(sct_info->timeout_cnt)=(sct_info->timeout_cnt)+contador_timer;
			control_state=(control_state ^ 0x40);	
			retorna=contador_char;
			contador_timer=0;		
			break;	
		}
		else if(contador_timer==(sct_info->resend_count)){
			printf("\nTIMEOUT\n");
			retorna=-1;
			break;
	 	}	
		else if(contador_bcc==(sct_info->resend_count)){
			printf("\nRecebidos 3 REJs consecutivos\n");	
			retorna=-1;				
			break;
		} 
	}
	free(full_frame);
	return retorna;
}

