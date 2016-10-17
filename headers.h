void conexao();
int llopen(int ports, char** argv); 
int llwrite(int fd,char *buffer,int lenght);
bool timeout_func(int *count,int control,int length, int fd);
int llread(int fd,char *buffer);
