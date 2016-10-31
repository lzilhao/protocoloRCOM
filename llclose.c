#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int num_timeouts=0;
int timeout=0;
void conexao2() {                 // atende alarme
	num_timeouts++;
	timeout=1;
}


int llclose(int fd, char* mode){
	unsigned char buf[255];
	unsigned char buf2[255];
	int i=0;
	int alm = 1;
	(void) signal(SIGALRM, conexao2);
	
	if (strcmp(mode, "T") == 0) {
		buf[0] = 0x7E; //flag
    	buf[1] = 0x03; //Emissor ou Receptor
    	buf[2] = 0x0b; //Controlo com byte DISC
    	buf[3] = (buf[1] ^ buf[2]);
    	buf[4] = 0x7E;
    	    	
    	while(num_timeouts<3){    	
			for (i=0; i<5; i++){
				write(fd, &(buf[i]), 1);				
			}			
			alarm(3);
			while(timeout == 0) {				
				read(fd, &(buf2[0]), 1);
				if (buf2[0] == 0x7e){
					for (i=1; i<5; i++){
						read(fd, &(buf2[i]), 1);
					}

					if(buf2[3] != (buf2[1] ^ buf2[2])) {
					//	printf("Bcc error, resending...\n");		//falta um contador para limitar num de erros
						buf2[0] = 0;
					}
					else if (buf2[2] == 0x0b){
						buf2[2] = 0x07;
						for (i=0; i<5; i++){
							write(fd, &(buf2[i]), 1);						
						}			
						close(fd);
						alarm(0);
						alm = 0;
						return 1;
					}
					else if (buf2[2] != 0x0b){
						//printf("Error: Did not receive DISC, resending..\n");						
					}							
				}				
			}
			if (alm == 0) break;
			timeout = 0;
    	}
    	if (num_timeouts == 3) {
    		close(fd);
    	}    	
	}
	
	else if (strcmp(mode, "R") == 0){
		while(num_timeouts<3){
			read(fd, &(buf[0]), 1);
			if (buf[0] == 0x7e){
				for (i=1; i<5; i++){
    				read(fd, &(buf[i]), 1);
    			}    				
    			if (buf[2] == 0x0b){
    				if (buf[3] != (buf[1] ^ buf[2])){
    					buf[0] = 0;
    					continue;
    				}
    				for (i=0; i<5; i++){
						write(fd, &(buf[i]), 1);
					}
					alarm(3);
					i=0;
					while(timeout == 0){
						read(fd, &(buf2[i]), 1);
						if (buf2[0] == 0x7e) {
							for (i=1; i<5; i++){
								read(fd, &(buf2[i]), 1);								
							}
							if (buf2[2] == 0x07){
								close(fd);
								return 1;
							}
							else {
								close(fd);
								return -1;
							}
						}
					}
					timeout = 0;					
    			}
    			else {
    				buf[0] = 0;
    			} 
			}
		}
		if (num_timeouts == 3) {
			close(fd);
			return -1;
		}
	}	    
    /*if (tcsetattr(fd, TCSANOW, & oldtio) == -1) {
	    perror("tcsetattr");
	    exit(-1);
	}*/	
	return -1;
	
}
