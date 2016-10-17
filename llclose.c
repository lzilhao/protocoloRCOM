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

void conexao2()                   // atende alarme
{
	printf("Timeout # %d\n", num_timeouts+1);
	num_timeouts++;
	timeout=1;
}


int llclose(int fd, char* mode){
	unsigned char buf[255];
	unsigned char buf2[255];
	int i=0;
	int alm = 1;
	(void) signal(SIGALRM, conexao2);
	
	if (strcmp(mode, "TRANSMITTER") == 0) {
		buf[0] = 0x7E; //flag
    	buf[1] = 0x03; //Emissor ou Receptor
    	buf[2] = 0x0b; //Controlo com byte DISC
    	buf[3] = (buf[1] ^ buf[2]);
    	buf[4] = 0x7E;
    	
    	
    	
    	while(num_timeouts<3){    	
			for (i=0; i<5; i++){
				write(fd, &(buf[i]), 1);				
				printf("Enviei %d %#08x\n", i, buf[i]);			
			}			
			alarm(3);
			while(timeout == 0) {				
				read(fd, &(buf2[0]), 1);
				if (buf2[0] == 0x7e){
					for (i=1; i<5; i++){
						read(fd, &(buf2[i]), 1);
						printf("Recebi %d %#08x\n", i, buf2[i]);				
					}

					if(buf2[3] != (buf2[1] ^ buf2[2])) {
						printf("Bcc error, resending...\n");		//falta um contador para limitar num de erros
						buf2[0] = 0;
					}
					else if (buf2[2] == 0x0b){
						buf2[2] = 0x07;
						for (i=0; i<5; i++){
							write(fd, &(buf2[i]), 1);						
						}			
						printf("Disconnecting...\n");
						close(fd);
						alarm(0);
						alm = 0;
						break;
					}
					else if (buf2[2] != 0x0b){
						printf("Error: Did not receive DISC, resending..\n");						
					}							
				}				
			}
			if (alm == 0) break;
			//printf("Timeout #%d\n", num_timeouts);
			timeout = 0;
    	}
    	if (num_timeouts == 3) {
    		printf("Erro, a fechar sem confirmacao...\n");
    		close(fd);
    	}
    	
	}
	
	else if (strcmp(mode, "RECEIVER") == 0){
		while(num_timeouts<3){
			read(fd, &(buf[0]), 1);
			if (buf[0] == 0x7e){
				for (i=1; i<5; i++){
    				read(fd, &(buf[i]), 1);
    				printf("Recebi %d %#08x\n", i, buf[i]); 					
    			}    				
    			if (buf[2] == 0x0b){
    				printf("Disconnect signal received\n");
    				if (buf[3] != (buf[1] ^ buf[2])){
    					printf("bcc error, re-reading\n");
    					buf[0] = 0;
    					continue;
    				}
    				for (i=0; i<5; i++){
						write(fd, &(buf[i]), 1);
						printf("Enviei %d %#08x\n", i, buf[i]);						
					}
					alarm(3);
					i=0;
					printf("Waiting for acknowledge....\n");
					while(timeout == 0){
						read(fd, &(buf2[i]), 1);
						if (buf2[0] == 0x7e) {
							for (i=1; i<5; i++){
								read(fd, &(buf2[i]), 1);								
							}
							if (buf2[2] == 0x07){
								printf("Acknowledge received\nDisconnecting...\n");
								close(fd);
								return 0;
							}
							else {
								printf("Error: Wrong acknowledge, closing...\n");
								close(fd);
								return -1;
							}
						}
					}
					timeout = 0;					
    			}
    			else {
    				printf("Error: Did not receive DISC byte, re-reading...\n");
    				buf[0] = 0;
    			} 
			}
		}
		if (num_timeouts == 3) {
			printf("Timeout number exceeded, closing without confirmation...\n");
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
