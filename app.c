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

#define TAMANHO_DADOS 6

int main(int argc, char** argv)
{

int state=0;
char *package;
int i;
int estado_escrita;
int size=6;
int fd;
package=(char*) malloc(size+4);

	if ( (argc < 3) || ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0) )) 
	{
		printf("Usage:\tnserial SerialPort Mode\n\tex: nserial /dev/ttyS1 TRANSMITTER\n");
		exit(1);
	}
	//int fd = open(argv[1], O_RDWR | O_NOCTTY );
	
	fd=llopen(3,argv);
	//if( (estado_ligacao>=0) && ( strcmp(argv[2],"TRANSMITTER") == 0) )
	if( ( strcmp(argv[2],"TRANSMITTER") == 0) )	
	{
		if(state==0) // enviar pacote de iniciação
		{
			
			package[0]=0x02; // codigo controlo para iniciar
			package[1]=0x00; // 0 é o tamanho do ficheiro, 1 é o nome
			package[2]=0x01; // tamanho do package[3] em bytes
			package[3]=0x06; // tamanho do ficheiro completo a enviar
			printf("\n pacote: %d",(int)strlen(package));

			estado_escrita=llwrite(fd,package,10);			
			if(estado_escrita>=0)
			{
				printf("\nSucesso\n");
				state=1;
			}
			else
			{
				printf("\nErro na escrita\n");
			}			
		}
	}
	else if( ( strcmp(argv[2],"RECEIVER") == 0) )
	{
		llread(fd,package);
		printf("\n Sou o receptor e tenho este package:\n");	
		for(i=0;i<(size+4);i++)
		printf("\n Indice %d: %#08x\n",i,package[i]);	

	}

return 1;
}
