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

#define TAMANHO_DADOS 40
int recep_int;
FILE *file=NULL;
int main(int argc,char** argv)
{

int write_state=0;
unsigned char *package;
unsigned char package_recep[5];
int i,j;
int estado_escrita;
int read_state=0;
char str_teste[2];
int flag_read=0;
int fd;
//int recep_int;
int read_return=10;
unsigned char *dados;
int tamanho_dados;
unsigned char nr_sequencia=0x00;
int size;
int tamanho_file;

char *enviar;

int tambytes, tambytesaux;
unsigned char leitura[TAMANHO_DADOS];
size_t nread;
int k=1;
int tamdadosaux;
int envios=0;
setbuf(stdout, NULL);
	if ( (argc < 3) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) )) 
	{
		printf("Usage:\tnserial SerialPort Mode\n\tex: nserial /dev/ttyS1 TRANSMITTER\n");
		exit(1);
	}
	//int fd = open(argv[1], O_RDWR | O_NOCTTY );
	
	fd=llopen(3,argv);

	
			
	if( ( strcmp(argv[2],"TRANSMITTER") == 0) )	
	{
	
			/******************Abrir ficheiro*******************/

		file = fopen("test.txt","r");
	
		if(file==NULL)
		{
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


		tambytesaux=tambytes;
	
		if(write_state==0) // enviar pacote de iniciação
		{
			k=1;
			while(tambytesaux/255>1)
			{
					k++;
					tambytesaux=tambytesaux/255;
			}
			tambytesaux=tambytes;

			package=(unsigned char*)malloc(k+3);
			
			package[0]=0x02; // codigo controlo para iniciar
			package[1]=0x00; // 0 é o tamanho do ficheiro, 1 é o nome
			package[2]=k; // tamanho do package[3] em bytes
			
			for(i=0;i<k;i++)
			{
					package[i+3]=tambytesaux;
				  	tambytesaux=tambytesaux>>8;	
			}
			
			//printf("Enviei em Package[0]: %#08x\nEnviei em Package[1]: %#08x\nEnviei em Package[2]: %#08x\nEnviei em Package[3]: %#08x\nEnviei em Package[4]: %#08x\n",package[0],package[1],package[2],package[3],package[4]);
			
			estado_escrita=llwrite(fd,package,3+k);			
			if(estado_escrita>=0)
			{
				printf("\nSucesso\n");
				write_state=1;
			}
			else
			{
				printf("\nErro na escrita\n");
			}			
		}
		if(write_state==1)
		{
			/******************Ler ficheiro**********************/
			
			printf("\nState 1\n");
		//	sleep(2);
			
			envios=tambytes/TAMANHO_DADOS;
			if(tambytes%TAMANHO_DADOS>0)
			{
				envios++;
			}
			
			for(i=0; i<envios;i++)
			 {
			  nread = fread(leitura, 1, sizeof (leitura), file) > 0;
			//	printf("%s\n",leitura);
				
				k=1;	
				tamdadosaux=TAMANHO_DADOS;
				
				package=(unsigned char*) malloc(strlen(leitura)+4);
				package[0]=0x01; // codigo controlo para dados
				package[1]=nr_sequencia; // nr_sequencia
				if( (tamdadosaux/255)>1 )
				{
					package[3]=0xff;                        // <------------- ALTERAR
					package[2]=(tamdadosaux >> 8);
					
				}
				else
				{
					package[3]=strlen(leitura);
					package[2]=0;
				}
				/*for(i=0;i<k;i++)
				{
					package[i+2]=tamdadosaux;
				  tamdadosaux=tamdadosaux>>8;	
				}	*/
				memcpy(&package[4],leitura,strlen(leitura));
				
			/*	for(i=0;i<strlen(leitura)+4;i++)
				{
						printf("Enviei em Dados: %#08x - - - - - - %c\n",package[i],package[i]);
				}*/
				//printf("\n length: %d\n",(strlen(leitura)+4));
			//	sleep(2);
				estado_escrita=llwrite(fd,package,strlen(leitura)+3);
				if(estado_escrita>=0)
			  {
					printf("\nSucesso\n");
					write_state=1;
			  }
			  else
				{
					printf("\nErro na escrita\n");
				}		
			//	sleep(1);
			}
    	if (ferror(file)) 
			{
        /* deal with error */
    	}
    	fclose(file);
		  /****************************************************/
		}
	free(package);

	}
	else if( (strcmp(argv[2],"RECEIVER") == 0) )
	{
		//for(j=0;j<2;j++)
		//{
			//printf("\n yoo %d\n",j);
			file=fopen("teste.jpg", "wb");
			if(file==NULL)
				printf("\n Erro na abertura do ficheiro\n");
			tamanho_file=0;
			flag_read=0;
			while(flag_read==0)
			{
				while(read_state==0)
				{
					read_return=llread(fd,package_recep);
						if((read_return!=-1) && (package_recep[0]==0x02))
						{
							printf("\n package[2]: %d\n",package_recep[2]);
							printf("\n package[3]: %d\n",package_recep[3]);
							printf("\n package[4]: %d\n",package_recep[4]);
							for(i=0;i<(int)package_recep[2];i++)
								{
								//	printf("\n tamanho file: %d\n",tamanho_file);
									if(i==0)
									{
										tamanho_file=tamanho_file+(unsigned int)package_recep[3+i];
										printf("\n Tamanho1: %d\n",tamanho_file);
									}
									else if(i>0)
									{
										tamanho_file=tamanho_file+(unsigned int)package_recep[3+i]*(256*i);								
										printf("\n Tamanho2: %d\n",tamanho_file);									
									}
								}
								//printf("\n tamanho file direto: %d\n",package_recep[3]);
								
								recep_int=tamanho_file/TAMANHO_DADOS;
								read_state=1;
								printf("\n tamanho file: %d\n",tamanho_file);
								//sleep(10);
								if( (tamanho_file % TAMANHO_DADOS>0) )
									recep_int++;
								/*	for(i=0;i<(5);i++)//tem que se alterar de forma a receber mais info
									printf("\n Indice %d: %#08x\n",i,package_recep[i]);	
								sleep(4);*/
								printf("\n recep_int: %d\n",recep_int);
								//sleep(10);
						}
						else
							read_state=-1;
				}
				while(read_state==1)
				{
					printf("\n Entrei no while files\n");
					read_return=llread(fd,leitura);
						if( (read_return>0) && (leitura[0]==0x01) )//files
						{

							tamanho_dados=((int)(leitura[3])+256*(int)(leitura[2]));
							dados=(unsigned char*) malloc( tamanho_dados );
							memcpy(dados,&(leitura[4]),tamanho_dados );
							printf("\n Olá da app: \n\n");
							fwrite(dados,tamanho_dados,1,file);
							for(i=0;i<tamanho_dados;i++)
							{
								printf("\nIndice: %d: %c\n",i,dados[i]);
							}								

							if(recep_int == 0)
							{
								read_state=2;
								flag_read=1;
							}
							else if(recep_int > 0)
							{
								i=1;
								printf("\n RECEP_INIT: %d\n",recep_int);
								while(i<recep_int)
								{
									read_return=llread(fd,leitura);
									if(read_return>0)
									{
										tamanho_dados=((int)(leitura[3])+256*(int)(leitura[2]));
										dados=(unsigned char*) malloc( tamanho_dados );
										memcpy(dados,&(leitura[4]),tamanho_dados );
										//printf("\n Olá da app: \n\n");
											for(j=0;j<tamanho_dados;j++)
											{
												printf("\nIndice: %d -----%#08x------ %c\n",j,dados[j],dados[j]);
											}											
										fwrite(dados,tamanho_dados,1,file);
										//fseek(file,tamanho_dados,SEEK_CUR);
										i++;
										read_state=2;
										flag_read=1;
										printf("\nESTE É O I: %d\n",i);					
									}
								}
							}
						}
						else if( (read_return>0) && (leitura[0]==0x03) ) //end
						{

						}
						else if(read_return==-1)
						{

						}
				}
				
				//printf("\npuntz\n");
				//sleep(2);
			/*	read_return=llread(fd,package_recep);
				if(read_return!=-1)
					break;*/
				//sleep(2);
			}
	printf("\n ACABOUU \n");
	fclose(file);
			//printf("\n Sou o receptor e tenho este package:\n");	
			//for(i=0;i<(size+4);i++)
		/*	printf("\n Indice %d: %#08x\n",0,package[0]);
				
			if(package[0]==0x01)
			{
				tamanho_dados=((int)(package[3]));
				dados=(unsigned char*) malloc( tamanho_dados );
				memcpy(dados,&(package[4]),tamanho_dados );
				printf("\n Olá da app: \n\n");
				for(i=0;i<tamanho_dados;i++)
				{
					printf("\nIndice: %d: %c\n",i,dados[i]);
				}
			}
			else if(package_recep[0]==0x02)
			{
				for(i=0;i<(4);i++)//tem que se alterar de forma a receber mais info
				printf("\n Indice %d: %#08x\n",i,package_recep[i]);	
								
					
			}*/
		/*	else if(package[0]==0x03)
			{


			}*/
		//}
 	free(dados);
	}


return 1;
}
