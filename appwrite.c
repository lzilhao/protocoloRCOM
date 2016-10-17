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

int main(int argc,char** argv)
{

int write_state=0;
unsigned char *package;
int i,j;
int estado_escrita;
char str_teste[2];

int fd;
int read_return=10;
char *dados;
int tamanho_dados;
unsigned char nr_sequencia=0x00;
int size;
int resto;
unsigned char *leitura_resto;
char *enviar;
FILE *file=NULL;
int tambytes, tambytesaux;
char leitura[TAMANHO_DADOS];
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

		file = fopen("teste.jpg","rb");
	
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

		//sleep(10);
		tambytesaux=tambytes;
	
		if(write_state==0) // enviar pacote de iniciação
		{
			k=1;
			while(tambytesaux/255>1)
			{
					k++;
					tambytesaux=(tambytesaux>>8);
			}
			tambytesaux=tambytes;

			package=(unsigned char*)malloc(k+4);
			
			package[0]=0x02; // codigo controlo para iniciar
			package[1]=0x00; // 0 é o tamanho do ficheiro, 1 é o nome
			package[2]=k; // tamanho do package[3] em bytes
			printf("\n K: %d",k);
			//sleep(10);
			for(i=0;i<k;i++)
			{
					package[i+3]=tambytesaux;
				    tambytesaux=tambytesaux>>8;	
			}
			
			//printf("Enviei em Package[0]: %#08x\nEnviei em Package[1]: %#08x\nEnviei em Package[2]: %#08x\nEnviei em Package[3]: %#08x\nEnviei em Package[4]: %#08x\n",package[0],package[1],package[2],package[3],package[4]);
			//printf("\nTamanho: %d\n",(package[3]+package[4]*256));
			//sleep(10);
			estado_escrita=llwrite(fd,package,3+k);			
			if(estado_escrita>=0)
			{
			//	printf("\nSucesso\n");
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
			/*if(tambytes%TAMANHO_DADOS>0)
			{
				envios++;
			}*/
			resto=tambytes-envios*TAMANHO_DADOS;
			fseek(file, 0, SEEK_SET);
			i=0;			
			while(i<envios)
			 {	
				printf("\nI: %d  Vs envios: %d\n",i,envios);
				nread = fread(leitura, 1,(sizeof(leitura)), file) ;
				//i=fseek(file, sizeof(leitura)-4, SEEK_CUR);
				//if(i!=0)
					//printf("\nFalha FSEEK\n"); 
			//	printf("%s\n",leitura);
				
				k=1;	
				tamdadosaux=TAMANHO_DADOS;
				
				package=(unsigned char*) malloc(sizeof(leitura)+4);
				package[0]=0x01; // codigo controlo para dados
				package[1]=nr_sequencia; // nr_sequencia
				if( (tamdadosaux/255)>1 )
				{
					package[3]=(tamdadosaux & 0x00ff);                        // <------------- ALTERAR
					package[2]=(tamdadosaux >> 8);
					
				}
				else
				{
					package[3]=sizeof(leitura);
					package[2]=0;
				}
				/*for(i=0;i<k;i++)
				{
					package[i+2]=tamdadosaux;
				  tamdadosaux=tamdadosaux>>8;	
				}	*/
				
				memcpy(&package[4],leitura,sizeof(leitura));
			/*	printf("sizeof(str):%d",sizeof(leitura));
				printf("strlen(leitura):%d",sizeof(leitura));
				printf("\npacakge[8]: %c\n",package[8]);
				printf("\npacakge[9]: %c\n",package[9]);
				printf("\n");*/
				/*for(i=4;i<strlen(leitura)+4;i++)
				{
						printf("\n%c--%#02x",package[i],package[i]);
				}*/
				//printf("\n");
				//printf("\n length: %d\n",(strlen(leitura)+4));
			//	sleep(1);
				/*for(i=0;i<sizeof(leitura)+4;i++)
				{			
					printf("\n Indice na APP %d: %#02x----  %c  \n",i,package[i],package[i]);
				}*/
				estado_escrita=llwrite(fd,package,sizeof(leitura)+4);
				if(estado_escrita>=0)
			  {
					//printf("\nSucesso\n");
					write_state=1;
					i++;
					
			  }
			  else
				{
					printf("\nErro na escrita\n");
				}		
			free(package);
			}	
			if(resto>0)	
			{	
				package=(unsigned char*) malloc(resto+4);	
				package[0]=0x01; // codigo controlo para dados
				package[1]=nr_sequencia; // nr_sequencia
				package[2]=0;
				package[3]=resto;
				leitura_resto=(unsigned char*) malloc(resto);	
					
				nread = fread(leitura_resto, 1,resto, file);
				memcpy(&package[4],leitura_resto,resto);
				estado_escrita=llwrite(fd,package,resto+4);	
				free(package);	
				free(leitura_resto);
				if(estado_escrita>=0)
			  {
					//printf("\nSucesso\n");
					write_state=1;
					
			  }
			  else
				{
					printf("\nErro na escrita\n");
				}		

			}
    	if (ferror(file)) 
			{
        /* deal with error */
    	}
    	fclose(file);
		  /****************************************************/
		}


	}
	else if( (strcmp(argv[2],"RECEIVER") == 0) )
	{
		for(j=0;j<2;j++)
		{
			printf("\n yoo %d\n",j);
			while(1)
			{
				printf("\npuntz\n");
				//sleep(2);
				read_return=llread(fd,package);
				if(read_return!=-1)
					break;
				//sleep(2);
			}
			//printf("\n Sou o receptor e tenho este package:\n");	
			//for(i=0;i<(size+4);i++)
			printf("\n Indice %d: %#08x\n",0,package[0]);
				
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
			else if(package[0]==0x02)
			{
				for(i=0;i<(4);i++)//tem que se alterar de forma a receber mais info
				printf("\n Indice %d: %#08x\n",i,package[i]);	
								
					
			}
		/*	else if(package[0]==0x03)
			{


			}*/
		}
 
	}
return 1;
}
