typedef struct info {
	int i_received;
	int i_sent;
	int package_lost;
	int rej_count;
	int time_out;
	int resend_count;
	int data_size;
	int timeout_cnt;
	
} infoo;

void conexao();
int llopen(int porta, char* mode, int my_baudrate);
int llwrite(int fd,unsigned char *buffer,int length,infoo *sct_info);
//bool timeout_func(int *count,int control,int length, int fd);
int llread(int fd,unsigned char *buffer,unsigned char *full_frame_stuffed);
int llclose(int fd, char* mode);

