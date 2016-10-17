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
#define REJ 0x01
#define RR 0x03
unsigned char ack=0x00;
int bcc_cnt=0;
unsigned char *byte_destuffing(unsigned char *str,int *size)
{
int i;
int j;
int k;
int *ponto_crit=(int *) malloc(50);
unsigned char *buffer=(unsigned char*) malloc((*size));
int hit=0;

for(i=0;i<(*size);i++)
{
	if(str[i]==0x7d)
	{
		ponto_crit[hit]=i;
		hit++;
	}
	memcpy(&buffer[i],&str[i],1);
	
}

str=(unsigned char*) realloc(str,(*size)-hit);
i=1;
j=0;
k=1;
while(i<((*size)))
{
	if(ponto_crit[j]==i)
	{	
		str[k]=str[i+1]^0x20;
		i=i+2;
		k++;
		j++;
	}
	else
	{
		memcpy(&str[k],&buffer[i],1);
		i++;
		k++;
	}
}
(*size)=(*size)-hit;
/*for(i=0;i<(*size);i++)
printf("\n NO byte destuffing indice: %d:  %#08x----%c\n",i,str[i],str[i]);*/
free(buffer);
free(ponto_crit);
return str;
}

int ack_check(unsigned char c)
{
int retorna;
char teste;
teste=(ack>>1);
	//printf("\n ACK  %#08x\n",teste);
	//printf("\n c  %#08x\n",c);
	
	//sleep(2);
	if((c^(ack>>1))==0x40)
		retorna=1;
	else
		retorna=0;
return retorna;
}


int bcc_check(unsigned char *str,unsigned char *str2,int c,int size)
{
int i;
unsigned char bcc;

int indice_final=size;
int retorna=1;
	if(c==1)//c=1 estamos a tratar o BCC1
	{
		bcc=str[1]^str[2];
		//printf("\n Str1: %#08x\n",str[1]);
		//printf("\n Str2: %#08x\n",str[2]);
		//printf("\n Str3: %#08x\n",str[3]);
		if(bcc==str[3])
			retorna=1;
		else
			retorna=0;
	}
	else if(c==2)//dados
	{
		bcc=str[0]^str[1];  //isto não bom o suficiente ... Pode ter mais que um L(L1 e L2)...
	//	printf("\n Indice 0:  %#08x\n",str[0]);
	//	printf("\n Indice 1:  %#08x\n",str[1]);
		//printf("\n Bcc recebido: %#08x\n",str2[0]);
		for(i=2;i<(indice_final);i++)
		{
			bcc=str[i]^bcc;
			//printf("\n Indice :%d  %#08x\n",i,str[i]);

		}
		if(bcc==str2[0])
			retorna=1;
		else
			retorna=0;
		
			printf("\n bcc :%#08x\n",bcc);
			printf("\n BCC2 :%#08x\n",str2[0]);	
	
	}	

//	sleep(3);	
return retorna;
}

int llread(int fd,unsigned char *buffer)
{
bool final_recep=FALSE;
int i,j,k;
int escreve;
int tamanho;
int l=-1;
int flag;
int nr_dados;
int nr_cmd=0;
int bcc1_cod,bcc2_cod,ack_cod;
int codigo; // 1 sucesso nos dados, 2 sucesso no start, 3 sucesso no end, 4 erro no bcc1, 5 erro no ack, 6 erro no bcc2
unsigned char *full_frame_stuffed=(unsigned char*) malloc(200);
unsigned char *full_frame;
unsigned char frame_resposta[5];
int retorna;
setbuf(stdout,NULL);

	final_recep=FALSE;
	printf("\n Siga ler\n");
	while (final_recep==FALSE) 
	{
//	printf("\n Frame header:  %#08x\n",full_frame_stuffed[0]);
        l=read(fd,&(full_frame_stuffed[0]),1);
		if ((full_frame_stuffed[0] == 0x7E) && (l>0)) 
		{
		//	printf("\nReceptor encontrou a flag inicial, vou ler\n");
		//	printf("\n Indice 0:  %#08x\n",full_frame_stuffed[0]);
			i=1;
			while( (full_frame_stuffed[i-1]!=0x7E) || (i==1) ) //preenche frame header
			{
				
				l=read(fd,&(full_frame_stuffed[i]),1);
			
				 	if(l>0)
					{
						//printf("\n Indice %d:  %#08x\n",i,full_frame_stuffed[i]);
						i++;													
					}
						
			}
		//printf("\nSAI 1\n");
		retorna=1;
		final_recep=true;
		}
	}
		tamanho=i;
		full_frame=byte_destuffing(full_frame_stuffed,&tamanho);
		/*for(j=0;j<tamanho;j++)
		printf("\n Indice pós destuffing %d:  %#08x------%c\n",j,full_frame[j],full_frame[j]);*/
		
		bcc1_cod=bcc_check(full_frame,NULL,1,0); // testa bcc1
		
		if(bcc1_cod==1)
		{
			ack_cod=ack_check(full_frame[2]);
			if(ack_cod==1)
			{
				if(full_frame[2]==0x0B)// é um DISC
				{
					//free((unsigned char*)full_frame);
					printf("\n DISC\n");
				}
				else
				{
					if(full_frame[4]==0x01)//dados
					{
						//printf("\n PARECE QUE É DADOS\n");
					//	sleep(2);
						//for(j=0;j<tamanho;j++)
						//printf("\n Indice %d:  %#08x\n",j,full_frame[j]);

						nr_dados=full_frame[6]*256+full_frame[7];
						bcc2_cod=bcc_check(&full_frame[8],&full_frame[tamanho-2],2,nr_dados );
						//free((unsigned char*)full_frame);
						if(bcc2_cod==1)
						{
							printf("\n BCC2 TOPE\n");
							//ack=ack^0x08;
							retorna=1;
							codigo=1;
						}
						else
						{
							bcc_cnt++;	
							printf("\n BCC2 MAL\n");
							if(bcc_cnt==3)
							{							
								for(j=0;j<tamanho;j++)
								printf("\n Indice %d:  %#08x\n",j,full_frame[j]);
								sleep(10);
							}
								codigo=6;
						}			
					}
					else if(full_frame[4]==0x02)//ini
					{
						//free((unsigned char*)full_frame);
						printf("\n PARECE QUE É INI\n");
						k=6;
						flag=0;
						while(flag==0)
						{
							nr_cmd=nr_cmd+full_frame[k];
							k=k+full_frame[k];
						/*	{
								printf("\n Testar Flag %d:  %#08x\n",k+1,full_frame[k+1]);
							}*/
							if(full_frame[k+1]==0x7E)
							{
								flag=1;
								//printf("\nEROO\n");
							}
							else
								k=k+2;

						}
					
						
						codigo=2;
					}
					
				}
			}
			else
			{	
				//free((unsigned char*)full_frame);
				printf("\n Erro no ack\n");
				sleep(5);
				codigo=5;
			}
		}
		else
		{
			//free((unsigned char*)full_frame);
			printf("\n ERRO BCC1 \n");
			codigo=4;
		}
		for(j=0;j<tamanho;j++)
		//printf("\n Indice pós destuffing %d:  %#08x-----%c\n",j,full_frame[j],full_frame[j]);

	escreve=0;
	if(codigo==1)//sucesso dados
	{
		bcc_cnt=0;
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | RR);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		ack=ack^0x80;
		retorna=1;
		escreve=1;
		//sleep(4);
	
	}
	else if(codigo==2)//sucesso start
	{
		
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | RR);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		ack=ack^0x80;
		retorna=1;
		escreve=1;
	}
	else if(codigo==3)//sucesso end
	{

		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=0x0B;
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		retorna=1;
		escreve=1;
	}
	else if(codigo==4)//erro bcc1
	{
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | REJ);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		retorna=-1;
		escreve=1;
	}
	else if(codigo==5)//erro ack
	{
		
		retorna=-1;
	}
	else if(codigo==6)//erro bcc2
	{
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | REJ);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		retorna=-1;
		escreve=1;
	}
	printf("\n codigo: %d -- retorna: %d\n",codigo,retorna);
	if( (retorna==1) && (codigo==1) )
	{
		memcpy(buffer,&full_frame[4],(nr_dados+4));
		retorna=(nr_dados+4);
	}
	else if( (retorna==1)  && (codigo==2) )
	{
		memcpy(buffer,&full_frame[4],5);
	
		retorna=1;
	}
		
	if(escreve==1)
	{
		for(i=0;i<5;i++)
		{
			//printf("\n Indice %d:  %#02x\n",i,full_frame[i]);
			l=write(fd,&(frame_resposta[i]),1);
			if(l==-1)
				perror("Error printed by perror");
		}
	}
printf("\nisto\n");
free((unsigned char*)full_frame_stuffed);
printf("\nRETOOOORNA:%d\n",retorna);
printf("\nAquilo\n");
return retorna;
}
