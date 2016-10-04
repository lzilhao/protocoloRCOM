/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

//#define BAUDRATE B38400
#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int conta=0;
int interr;
volatile int STOP=FALSE;

void conexao()                   // atende alarme
{
	printf("Timeout # %d\n", conta+1);
	interr=1;
	conta++;
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i;
	int alm=1;
    
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
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



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
	
  printf("Write a message: ");

  /************************************************************/

	printf("A Estabelecer conexão...\n");
	

	(void) signal(SIGALRM, conexao);
	while(alm==1 && conta<3)
	{   
		interr=0;
		res=write(fd,"a",c+1);
	  	i=0;
		  //while (STOP==FALSE) {
			  alarm(3);       /* loop for input */
			  res = read(fd,&(buf[i]),1);   /* returns after 5 chars have been input */
			  if (buf[i]=='a') {
			   alm=0;
			   interr=1;				
			   i=0;
				alarm(0);       
			  }
		if(interr==0)
			{
				printf("A espera de resposta...\n");
				while(interr==0)
				{
				 res = read(fd,&(buf[i]),1);
					if(buf[i]=='a')
						{
							alarm(0);
							alm=0;
							interr=1;
						}
				}
			
			}			
		
	}


	if(conta<3)
	{
    	printf("Conexão estabelecida!\n");
	}	
	else if(conta>=3)
	{
		printf("Falha na ligação\n");
	}
  /************************************************************/

  /* 
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
    o indicado no guião 
  */


	sleep(1);
   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
