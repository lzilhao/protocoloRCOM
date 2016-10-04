/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
	int alm = 1; //Flag de alarme
	int i=1;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    printf("New termios structure set\n");


	/************************************************************/
	printf("A espera de transmissao\n");
	while(1){
		res=read(fd, &(buf[0]), 1);
		if (buf[0] == 0x7E)
		{
			printf("Encontrou a flag inicial, vou ler\n");
			while(i<=4)
			{
				(void)read(fd,&(buf[i]),1);
				printf("%#08x\n",buf[i]);
				i++;
			}
			//printf("Recebi em 0: %#08x\nRecebi em 1: %#08x\nRecebi em 2: %#08x\nRecebi em 3: %#08x\nRecebi em 4: %#08x\n",buf[0],buf[1],buf[2],!buf[3],buf[4]);
			buf[2]=0x01;
			write(fd, buf, 5);			
			break;	
		}
	}	

	/************************************************************/
/*
    int i = 0;
    while (STOP==FALSE) {       // loop for input 
      res = read(fd,&(buf[i]),1);   // returns after 5 chars have been input 
      i++;
      if (buf[i-1]=='\0') {
       STOP=TRUE;
       i=0;       
      }
      res = strlen(buf);
    }
    printf("Mensagem recebida:%s\nBytes recebidos:%d\n", buf, res+1);
*/
	/************************************************************/
/*
	//gets(buf);
	c=strlen(buf);
	res=write(fd,buf,c+1);
	printf("Mensagem enviada:%s\nBytes enviados:%d\n", buf, res);
*/

	/************************************************************/

  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
