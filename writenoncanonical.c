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

int conta=1;
int teste=0;
volatile int STOP=FALSE;

void conexao()                   // atende alarme
{
	printf("Timeout # %d\n", conta);
	conta++;
	teste=1;
}

int main(int argc, char** argv)
{
    int fd,c=1, res;
    struct termios oldtio,newtio;
    char buf[255];
    char buf2[255];
    int i=0;
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

  /************************************************************/

    buf[0]=0x7E;
	buf[1]=0x03;
	buf[2]=0x03;
	buf[3]= ((buf[1]^buf[2])-1);
	buf[4]=0x7E;
	c=strlen(buf);

  /************************************************************/

	i=1;

	printf("A Estabelecer conexão\n");
	
	(void) signal(SIGALRM, conexao);
	
	res=write(fd,buf,c);
	printf("Enviei em 0: %#08x\nEnviei em 1: %#08x\nEnviei em 2: %#08x\nEnviei em 3: %#08x\nEnviei em 4: %#08x\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
	alarm(3);

	while(alm==1)
	{   
		if(teste==1)
		{
			alarm(3);
			if (conta>3)
				break; 	
			res=write(fd,buf,c);
			printf("Enviei em 0: %#08x\nEnviei em 1: %#08x\nEnviei em 2: %#08x\nEnviei em 3: %#08x\nEnviei em 4: %#08x\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
			teste=0; 
		}

		res = read(fd,&(buf2[0]),5); 

		if (buf2[0]==0x7E) 
		{
			printf("Encontrou Flag incial, vou ler\n");
			while (i<=4) 
			{
                newtio.c_cc[VMIN] = 1;
                (void) read(fd, &(buf2[i]), 1);
                i++;
			}
			printf("Li em 0: %#08x\nLi em 1: %#08x\nLi em 2: %#08x\nLi em 3: %#08x\nLi em 4: %#08x\n",buf2[0],buf2[1],buf2[2],buf2[3],buf2[4]);
			alarm(0);			
			alm=0;							
			break;      
		}
	}

	if(buf2[2]==0x01)
	{
    	printf("Conexão estabelecida!\n");
	}	
	else 
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
