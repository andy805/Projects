
#ifndef __NETWORKS_H__
#define __NETWORKS_H__
#define BACKLOG 10
#define MAXBUF 1024
#define TIME_IS_NULL 1
#define TIME_IS_NOT_NULL 2
#define HEADER 3
#define FLAG_1 1
#define FLAG_2 2
#define FLAG_3 3
#define FLAG_4 4
#define FLAG_5 5
#define FLAG_6 6
#define FLAG_7 7
#define FLAG_8 8
#define FLAG_9 9
#define FLAG_10 10
#define FLAG_11 11
#define FLAG_12 12
#define FLAG_13 13


// for the server side
int tcpServerSetup(int portNumber);
int tcpAccept(int server_socket, int debugFlag);

// for the client side
int tcpClientSetup(char * serverName, char * port, int debugFlag);

int selectCall(int socketNumber, int seconds, int microseconds, int timeIsNotNull);

void addPDU(uint16_t *length, char* packet, char *data, int data_len);

void send_header(char flag, int socket_number);

//void read_packet(command_line_struct *cmd, char* packet);


#endif
