#define MAX_HANDLE 101
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>

typedef struct command_line_struct
{
    uint16_t total_length;
    uint8_t flag_command;
    uint8_t num_handles;
    int message_length;
    uint8_t source_len;
    char source_handle[MAX_HANDLE];
    char handle1[MAX_HANDLE];
    char handle2[MAX_HANDLE];
    char handle3[MAX_HANDLE];
    char handle4[MAX_HANDLE];
    char handle5[MAX_HANDLE];
    char handle6[MAX_HANDLE];
    char handle7[MAX_HANDLE];
    char handle8[MAX_HANDLE];
    char handle9[MAX_HANDLE];
    char message[1024];
    char in_buf[1400];
}command_line_struct;

void save_handle(command_line_struct *input, int number, char* handle);

int command_read(char* command);

void parse_command_line(int argc, char * argv[], char** handle_name, char** server_name, int *server_port);

void num_args_client(int argc);

void check_handle_name(char * name);

void print_cmd_struct(command_line_struct* cmd);

void save_message(char* buf, command_line_struct* cmd, char** pointer);

void save_text_only(char *buf, command_line_struct *cmd);

void process_input(command_line_struct* input, char *sendBuf);

void set_up_server(int socket_number, char* handle_name);

void pdu_client_setup(command_line_struct* cmd, char *client_h, int sd);

char* get_handle(command_line_struct* cmd, int handle);

void read_packet(command_line_struct *cmd, char* packet);
void add_null(char* buf, char *s, int len);

void send_flag4(command_line_struct *cmd, char *handle_name, int sd);
void send_flag7(command_line_struct *cmd, int socket_number, int handle_num);
void send_packet12(char *handle_name, int sd);
void send_packet11(int tot, int sd);

void read_packet4(char* buf, uint16_t len, char flag);
void save_packet4(char*buf, uint16_t len, char flag, char** messbuf, char** handlebuf);
void read_packet7(char * buf, uint16_t len, char flag);
int read_packet11(char * buf, uint16_t len, char flag);
void read_packet12(char * buf, uint16_t len, char flag);
