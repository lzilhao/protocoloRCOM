#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "headers.h"

#define MODEMDEVICE "/dev/ttyS1"

#define BAUDRATE1 B9600
#define BAUDRATE2 B19200
#define BAUDRATE3 B38400
#define BAUDRATE4 B115200

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


int conta=1;
int teste=0;
volatile int STOP=FALSE;

void conexao(){             // atende alarme
	printf("Timeout # %d\n", conta);
	conta++;
	teste=1;
}
int llopen(int porta, char* mode, int my_baudrate)
{
	int fd;
	struct termios oldtio,newtio;
	char buf[255];
	char buf2[255];
	int i=1;
	int alm=1;
	int j=0;
	int retorna;
	char serial[12];

	sprintf(serial, "/dev/ttyS%d", porta);

	printf("%s\n",serial);	

	/*
	Open serial port device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
	*/

	fd = open(serial, O_RDWR | O_NOCTTY );
	if (fd <0) { perror(serial); exit(-1); }

	if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
	perror("tcgetattr");
	exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	switch(my_baudrate)
	{
		case 1:
		newtio.c_cflag = BAUDRATE1 | CS8 | CLOCAL | CREAD;
		break;
		case 2:
		newtio.c_cflag = BAUDRATE2 | CS8 | CLOCAL | CREAD;
		break;
		case 3:
		newtio.c_cflag = BAUDRATE3 | CS8 | CLOCAL | CREAD;
		break;
		case 4:
		newtio.c_cflag = BAUDRATE4 | CS8 | CLOCAL | CREAD;
		break;
		default:
		return -1;
	}
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */


	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	//printf("New termios structure set\n");

	if (strcmp("T", mode) == 0) {
	/************************************************************/

	buf[0] = 0x7E; //flag
	buf[1] = 0x03; //Emissor ou Receptor
	buf[2] = 0x03; //Controlo
	buf[3] = (buf[1] ^ buf[2]);
	buf[4] = 0x7E;

	/************************************************************/

	i = 1;

	printf("A Estabelecer conexão\n");

	(void) signal(SIGALRM, conexao);

	j=0;
	while (j <= 4){
		write(fd, &(buf[j]), 1);
		j++;
	}
	alarm(3);

	while (alm == 1) {
		if (teste == 1) {
			alarm(3);
			if (conta > 3) break;
			j=0;
			while (j <= 4){
				write(fd, &(buf[j]), 1);
				j++;
			}
		    teste = 0;
		}
		read(fd, & (buf2[0]), 1);
		if (buf2[0] == 0x7E) {
		    while (i <= 4) {
		        (void) read(fd, & (buf2[i]), 1);
		        i++;
		    }
		    if( buf2[3] != (buf2[1]^buf2[2])) {			//testa o bcc
				i=1;				
				teste = 1;
			}
			else {
				alm = 0;
				alarm(0);
			}			
		}
	}
	
	if ((buf2[2] == 0x07)) {
		printf("Conexão estabelecida!\n");
		retorna=fd;
	} else {
		retorna=-1; //Falha na ligacão
	}

	/************************************************************/

	} else if (strcmp("R", mode) == 0) {
			printf("A espera de transmissao\n");		
			while (1) {
				read(fd, &(buf[0]), 1);
				if (buf[0] == 0x7E) {
				//	printf("Encontrou a flag inicial, vou ler\n");
					while (i <= 4) {					
						(void) read(fd, & (buf[i]), 1);
						//printf("%d %#08x\n",i , buf[i]);
						i++;
					}				

					if( buf[3] != (buf[1]^buf[2])) {			//testa o bcc
						//printf("erro no bcc\n");
						i=1;
						buf[0] = 0;							//se for diferente, reinicia a contagem e a flag
					}
					else {
						buf[2] = 0x07;
						buf[3] = (buf[1] ^ buf[2]);
						j=0;
						while(j < strlen(buf)){
							write(fd, &(buf[j]), 1);
							j++;
						}
						printf("Ligação estabelecida\n\n\n\n");
						retorna=fd;
						break;
					}								
				}
			}
		}
	return retorna;	
}
