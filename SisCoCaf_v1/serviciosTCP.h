int tcp_open(int);
int tcp_bind_listen();
int tcp_accept();
int tcp_read(char*, int);
int tcp_write(char*, int);
int tcp_closeFD();
int tcp_closeSOC();

void* tcp_servidor( void* );
void* tcp_cliente( void* );
 
