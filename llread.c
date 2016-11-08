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
#include <time.h>

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define REJ 0x01
#define RR 0x05
unsigned char ack=0x00;
int bcc_cnt=0;

int rand_err(int err_p){

	int i,c;
	int vec[20];

	for(i=0;i<(err_p/10);i++){
		vec[i]=1;;
	}
	for(i=(err_p/10);i<10;i++){
		vec[i]=0;
	}
	c=rand()%10;

	return vec[c];
}

unsigned char *byte_destuffing(unsigned char *str,int *size){
	int i;
	int j;
	int k;

	int *ponto_crit=(int *) malloc(*size);
	unsigned char *buffer=(unsigned char*) malloc((*size));
	int hit=0;

	for(i=0;i<(*size);i++){
		if((str[i]==0x7d) && (i!=(*size)-2)){
			ponto_crit[hit]=i;
			hit++;
		}
		memcpy(&buffer[i],&str[i],1);
	}
	i=1;
	j=0;
	k=1;
	while(i<((*size))){
		if((ponto_crit[j]==i) && (j<hit)){	
			str[k]=str[i+1]^0x20;
			i=i+2;
			k++;
			j++;
		}
		else{
			memcpy(&str[k],&buffer[i],1);
			i++;
			k++;
		}
	}
	(*size)=(*size)-hit;
	free(buffer);
	free(ponto_crit);
	return str;
}

int ack_check(unsigned char c){
	int retorna;
	//printf("ACK: %08x\n");
	//printf("c: %08x\n",c);
	if((c^(ack>>1))==0x40)
		retorna=1;
	else
		retorna=0;
	return retorna;
}


int bcc_check(unsigned char *str,unsigned char *str2,int c,int size){
	int i;
	unsigned char bcc;

	int indice_final=size;
	int retorna=1;
	bcc=0;
	if(c==1){   //c=1 estamos a tratar o BCC1
		bcc=str[1]^str[2];
		if(bcc==str[3])
			retorna=1;
		else
			retorna=0;
	}
	else if(c==2){     //dados
		bcc=str[0]^str[1];  //isto não bom o suficiente ... Pode ter mais que um L(L1 e L2)...
		for(i=2;i<(indice_final);i++){
			bcc=str[i]^bcc;
		}
		if(bcc==str2[0]){
			retorna=1;
		}
		else{
			retorna=0;
		}
	}	
	return retorna;
}

int llread(int fd,unsigned char *buffer,unsigned char *full_frame_stuffed){
	bool final_recep=FALSE;
	int i,k;
	int escreve;
	int tamanho;
	int l=-1;
	int flag;
	int nr_dados;
	int nr_cmd=0;
	int nr_cmd_dif;
	int bcc1_cod,bcc2_cod,ack_cod;
	int codigo; // 1 sucesso nos dados, 2 sucesso no start, 3 sucesso no end, 4 erro no bcc1, 5 erro no ack, 6 erro no bcc2
	//unsigned char *full_frame_stuffed=(unsigned char*) malloc(300);

	unsigned char *full_frame;
	unsigned char frame_resposta[5];
	int retorna;
	setbuf(stdout,NULL);

	final_recep=FALSE;
	while (final_recep==FALSE) {
        l=read(fd,&(full_frame_stuffed[0]),1);
		if ((full_frame_stuffed[0] == 0x7E) && (l>0)){
			i=1;
			while((full_frame_stuffed[i-1]!=0x7E)||(i==1)){   
				l=read(fd,&(full_frame_stuffed[i]),1);
				if(l>0){			
					i++;													
				}			
			}
			retorna=1;
			final_recep=true;
		}
	}	
	tamanho=i;	
	full_frame=byte_destuffing(full_frame_stuffed,&tamanho);
	bcc1_cod=bcc_check(full_frame,NULL,1,0); // testa bcc1
	if(bcc1_cod==1){
		ack_cod=ack_check(full_frame[2]);
		if(ack_cod==1){
			if(full_frame[2]==0x0B){  // é um DISC
				printf("\n DISC\n");
			}
			else{
				if(full_frame[4]==0x01){   //dados
					nr_dados=full_frame[6]*256+full_frame[7];
					if(rand_err(10))
						full_frame[tamanho-3]=full_frame[tamanho-3]^full_frame[tamanho-3];	
					bcc2_cod=bcc_check(&full_frame[4],&full_frame[tamanho-2],2,nr_dados+4 );
					if(bcc2_cod==1){
						retorna=1;
						codigo=1;
					}
					else{
						bcc_cnt++;	
						//printf("\n BCC2 MAL\n");
						codigo=6;
					}			
				}
				else if((full_frame[4]==0x02) || (full_frame[4]==0x03) ){   //ini
					k=6;
					flag=0;
					nr_cmd=0;
					nr_cmd_dif=1;
					while(flag==0){
						nr_cmd=nr_cmd+full_frame[k];
						k=k+full_frame[k];
						if(full_frame[k+1]==0x7E){
							flag=1;
						}
						else{	
							k=k+2;
							nr_cmd_dif++;
						}
					}
					codigo=2;
				}	
			}
		}
		else{	
			//printf("\n Erro no ack\n");
			codigo=5;
		}
	}
	else{
		//printf("\n ERRO BCC1 \n");
		codigo=4;
	}

	escreve=0;
	if(codigo==1){   //sucesso dados
		//bcc_cnt=0;
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | RR);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		ack=ack^0x80;
		retorna=1;
		escreve=1;

	}
	else if(codigo==2){     //sucesso start
		
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | RR);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		ack=ack^0x80;
		retorna=1;
		escreve=1;
	}
	else if(codigo==3){     //sucesso end

		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=0x0B;
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		retorna=1;

		escreve=1;
	}
	else if(codigo==4){     //erro bcc1
		//sleep(0.01);
		tcflush(fd, TCIOFLUSH);
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | REJ);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		retorna=-1;
		escreve=1;
	}
	else if(codigo==5){     //erro ack
		//tcflush(fd, TCIOFLUSH);
		//printf("erro ack\n");
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | RR);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		//sack=ack^0x80;
		escreve=1;
		retorna=-2;
	}
	else if(codigo==6){    //erro bcc2
		
		tcflush(fd, TCIOFLUSH);
		frame_resposta[0]=0x7E;
		frame_resposta[1]=0x01;
		frame_resposta[2]=(ack | REJ);
		frame_resposta[3]=0x00;
		frame_resposta[4]=0x7E;
		retorna=-1;
		escreve=1;
	}
	if((retorna==1) && (codigo==1)){
		memcpy(buffer,&full_frame[4],(nr_dados+5));
		retorna=(nr_dados+4);
	}
	else if((retorna==1)  && (codigo==2)){
		memcpy(buffer,&full_frame[4],(1+nr_cmd+2*nr_cmd_dif));
		retorna=1;
	}

	if(escreve==1){
		for(i=0;i<5;i++){
			l=write(fd,&(frame_resposta[i]),1);
			if(l==-1){
				perror("Error printed by perror");
			}
		}
	}
	//if(retorna==-2)
//		printf("AHHHH\n");
//	printf("something - %d\n", buffer[1]);

	return retorna;
}
