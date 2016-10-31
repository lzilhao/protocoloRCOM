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
#include "headers.h"

//#define TAMANHO_DADOS 1000
int recep_int;
FILE *file=NULL;

int pot(int x,int n)
{
int res;
int i;
res=x;
	for(i=1;i<n;i++)
		{
			res=res*x;
		}
return res;
}

int main(int argc,char** argv)
{
char percent = '%';
int write_state=0;
unsigned char *package;
unsigned char *package_recep;
int i,j;
int lendo;
int estado_escrita;
int read_state=0;
unsigned char *full_frame;
int flag_read=0;
infoo info_struct;
int fd;
int read_return=10;
unsigned char *dados;
int tamanho_dados;
unsigned char nr_sequencia=0x00;
int tamanho_file;
int tamanho;
int resto;
int escrita;
unsigned char *leitura_resto;
int byte_counter;
FILE *file=NULL;
int tambytes, tambytesaux;
unsigned char *leitura;
int k=1;
int tamdadosaux;
int envios=0;
int my_baudrate=0;

setbuf(stdout, NULL);


	/**************Testar Input***************/
	if(argc < 6){
		printf("Usage: nserial mode BUFFER_SIZE RESEND_COUNT TIMEOUT_TIME\n");
		printf("Example: 0 T 1000 3 3\n");
		exit(1);
	}
	else if((strcmp(argv[2], "T") != 0) && (strcmp(argv[2], "R") != 0)) {
		printf("Mode must be T for Transmitter or R for Receiver\n");
		exit(1);
	}
	/******************************************/


	/************Definir Estrutura*************/
	if((char)argv[3][0]=='0')
		info_struct.data_size=500;
	else
		info_struct.data_size=atoi(argv[3]);

	if((char)argv[4][0]=='0')
		info_struct.resend_count=3;
	else
		info_struct.resend_count=atoi(argv[4]);

	if((char)argv[5][0]=='0')
		info_struct.time_out=3;
	else
		info_struct.time_out=atoi(argv[5]);

	info_struct.i_received=0;

	info_struct.i_sent=0;

	info_struct.package_lost=0;

	info_struct.rej_count=0;	
	/********************************************/


	/************Definir Baudrate****************/
	printf("1- B9600\n");
	printf("2- B19200\n");
	printf("3- B38400\n");
	printf("4- B115200\n");
	printf("Introduza o o número da baudrate pretendida: ");
	scanf("%d",&my_baudrate);
	printf("\n");
	if(my_baudrate>4 || my_baudrate<1){
		printf("Número invalido\n");
		return -1;
	}
	/********************************************/


	/*************Iniciar Ligação****************/
	fd=llopen(argv[1][0]-48,argv[2],my_baudrate);
	if(fd==-1){
		printf("Erro a estabelecer conexão\n");
		return -1;
	}
	/********************************************/
	

	/*******************************************Transmissor***************************************/		
	if( strcmp(argv[2],"T") == 0){
		/******************Abrir ficheiro*******************/
		file = fopen("teste.jpg","rb");
		
		if(file==NULL)	{
		    printf("\n Erro a abrir ficheiro\n");
		    return -1;
		}
	
		printf("Ficheiro encontrado\n");
		/****************************************************/


		/**********Encontrar tamanho do ficheiro*************/
		fseek(file, 0L, SEEK_END);
		tambytes = ftell(file);
		rewind(file);
		printf("Tamanho: %d\n",tambytes);
		/****************************************************/

		escrita=1;
		tambytesaux=tambytes;

		/*********************Transmissão********************/
		while(escrita==1){
			/*********************Pacote de Controlo********************/
			if(write_state==0){
				k=1;
				while(tambytesaux/255>=1){
					k++;
					tambytesaux=(tambytesaux>>8);
				}
				tambytesaux=tambytes;

				package=(unsigned char*)malloc(k+4);
			
				package[0]=0x02; // codigo controlo para iniciar
				package[1]=0x00; // 0 é o tamanho do ficheiro, 1 é o nome
				package[2]=k; // tamanho do package[3] em bytes
				printf("\n K: %d",k);
				for(i=0;i<k;i++){
					package[i+3]=tambytesaux;
					tambytesaux=tambytesaux>>8;
				}
				estado_escrita=llwrite(fd,package,3+k,&info_struct);

				if(estado_escrita>=0){
					write_state=1;
				}
				else{
					printf("\nErro na escrita\n");
				}
				free(package);			
			}
			/*************************************************************/

			/*********************Pacotes do ficheiro*********************/
			else if(write_state==1){
				printf("\nState 1\n");
				tamanho=info_struct.data_size;
				envios=tambytes/tamanho;
				resto=tambytes-envios*tamanho;
				fseek(file, 0, SEEK_SET);
				i=0;
				leitura=(unsigned char*) malloc(tamanho);
				package=(unsigned char*) malloc(tamanho+4);
				package[1]=0;		
				while(i<envios){	
					fread(leitura, 1,tamanho, file) ;
					
					k=1;	
					tamdadosaux=tamanho;
					
					package[0]=0x01; // codigo controlo para dados
					package[1]=package[1]+1; // nr_sequencia
					if( (tamdadosaux/255)>=1 ){
						package[3]=(tamdadosaux & 0x00ff);                     
						package[2]=(tamdadosaux >> 8);	
					}
					else{
						package[3]=tamanho;
						package[2]=0;
					}
					memcpy(&package[4],leitura,tamanho);
			
					estado_escrita=llwrite(fd,package,tamanho+4,&info_struct);
					if(estado_escrita>=0){
						write_state=1;
						i++;
					}
				  	else{
						printf("\nErro na escrita\n");
						write_state=3;
						break;
					}		
				}
				free(package);

				if(resto>0)	{	
					package=(unsigned char*) malloc(resto+4);	
					package[0]=0x01; // codigo controlo para dados
					package[1]=nr_sequencia; // nr_sequencia
					if( (resto/255)>=1 ){
						package[3]=(resto & 0x00ff);
						package[2]=(resto >> 8);
						
					}
					else{
						package[3]=resto;
						package[2]=0;
					}
					printf("\n Resto: %d\n",resto);
					leitura_resto=(unsigned char*) malloc(resto);	
						
					fread(leitura_resto, 1,resto, file);
					memcpy(&package[4],leitura_resto,resto);	
					estado_escrita=llwrite(fd,package,resto+4,&info_struct);	
					free(package);	
					free(leitura_resto);
					if(estado_escrita>=0){
						write_state=2;	
					}
					else{
						printf("\nErro na escrita\n");
					}		
				}
			    if (ferror(file)){
					return -1;
				}
			    fclose(file);
			}
			/*************************************************************/

			/*****************Ultimo pacote do ficheiro*******************/

			else if(write_state==2){
				k=1;
				while(tambytesaux/255>=1){
					k++;
					tambytesaux=(tambytesaux>>8);
				}
				tambytesaux=tambytes;

				package=(unsigned char*)malloc(k+4);
				
				package[0]=0x03; // codigo controlo para iniciar
				package[1]=0x00; // 0 é o tamanho do ficheiro, 1 é o nome
				package[2]=k; // tamanho do package[3] em bytes
				printf("\n K: %d",k);			
				estado_escrita=llwrite(fd,package,3+k,&info_struct);

				if(estado_escrita>=0){
					printf("\nSucesso close pack\n");
					write_state=3;
				}
				else{
					printf("\nErro na escrita\n");
				}
				free(package);				
			}
			/*************************************************************/

			/**********************Terminar Ligação***********************/
			else if(write_state==3){
					info_struct.package_lost=info_struct.i_received-info_struct.i_sent;
					printf("\nEnvios total: %d \t Recepções: %d \t REJs: %d \t packets lost: %d\n",info_struct.i_received,info_struct.i_sent,info_struct.rej_count,info_struct.package_lost);  
					printf("\nNUMERO DE REJ: %d\n",info_struct.rej_count);
					estado_escrita=llclose(fd,argv[2]);
					if(estado_escrita>0)
						printf("\n Fechou com sucesso\n");
					else
						printf("\n Erro no close\n");
					escrita=0;
			}
			/*************************************************************/
		}
		/****************************************************/

		/****************Libertar memoria********************/
		free(leitura);
		/****************************************************/
	}

	/*******************************************Receptor***************************************/		
	else if( (strcmp(argv[2],"R") == 0) )
	{
		tamanho=atoi(argv[3]);
		byte_counter=0;
		leitura=(unsigned char*) malloc(tamanho);
		package_recep=(unsigned char*) malloc(tamanho+4);
		full_frame=(unsigned char*) malloc((tamanho+4)*2);
		read_state=0;
		file=fopen("teste.jpg", "wb");
		if(file==NULL){
			printf("\n Erro na abertura do ficheiro\n");
		}
		tamanho_file=0;
		flag_read=0;
		while(flag_read==0){
			if(read_state==0)
			{					
				read_return=llread(fd,package_recep,full_frame);
				if((read_return!=-1) && (package_recep[0]==0x02)){
					for(i=0;i<(int)package_recep[2];i++){
						if(i==0){
							tamanho_file=tamanho_file+(unsigned int)package_recep[3+i];
						}
						else if(i>0){
							tamanho_file=tamanho_file+(unsigned int)package_recep[3+i]*pot(256,i);								
						}
					}
				}
				recep_int=tamanho_file/tamanho;
				read_state=1;					
				if(tamanho_file % tamanho>0){
					recep_int++;
				}
			}
			else if(read_state==1){
				read_return=llread(fd,leitura,full_frame);
				if( (read_return>0) && (leitura[0]==0x01) ){
					tamanho_dados=((int)(leitura[3])+256*(int)(leitura[2]));
					dados=(unsigned char*) malloc( tamanho_dados );
					memcpy(dados,&(leitura[4]),tamanho_dados );
					fwrite(dados,tamanho_dados,1,file);
					byte_counter=byte_counter+tamanho_dados;				
					free(dados);
					if(recep_int == 0){
						read_state=2;
						flag_read=1;
					}
					else if(recep_int > 0){
						i=1;
						lendo=1;
						j=0;
						while(lendo==1){
							read_return=llread(fd,leitura,full_frame);
							if(read_return>0){
								if(leitura[0]==0x01){
									tamanho_dados=((int)(leitura[3])+256*(int)(leitura[2]));
									dados=(unsigned char*) malloc( tamanho_dados );
									memcpy(dados,&(leitura[4]),tamanho_dados );
									fwrite(dados,tamanho_dados,1,file);
									byte_counter=byte_counter+tamanho_dados;
									i++;
									free(dados);
								}
								else{
									lendo=0;
								}							
								j=0;
								printf("[");
								while(j<((i*10)/recep_int)){									
									printf("#");
									j++;
								}
								while(j<10){
									printf("_");
									j++;
								}
								printf("]");
								printf("%d%c",((i*100)/recep_int),percent);
								printf("\r");
							}	
						}
						read_state=2;
					}
				}
				else if(read_return==-1)
				{
					return -1;
				}	
			}
			else if(read_state==2){	
				read_return=llclose(fd,argv[2]);
				flag_read=1;
			}	
		}
		free(full_frame);
		fclose(file);
		printf("\n");
		printf("\nTamanho original do ficheiro: %d Bytes\n",tamanho_file);
		printf("\nTotal de bytes recebidos: %d Bytes\n",byte_counter);
	}	
	printf("\n");
	return 1;
}