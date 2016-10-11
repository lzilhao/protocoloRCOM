#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "llopen.h"

int main(int argc, char** argv)
{
	    if ( (argc < 3) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort Mode\n\tex: nserial /dev/ttyS1 TRANSMITTER\n");
      exit(1);
    }
	
	llopen(3,argv);
return 0;
}
