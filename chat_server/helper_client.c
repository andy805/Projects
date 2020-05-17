#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helper_client.h"
#include "networks.h"
/*
int main(int argc, char* argv[])
{
    char * handle_name = NULL;
    char * server_name = NULL;
    int server_port = -1;
    parse_command_line(argc, argv, &handle_name, &server_name, &server_port);
    return 0;
    
    
}
*/

void parse_command_line(int argc, char * argv[], char** handle_name, char** server_name, int *server_port)
{
    num_args_client(argc);
    *handle_name = argv[1];
    check_handle_name(*handle_name);
    *server_name = argv[2];
    int port = atoi(argv[3]);
    *server_port = port;
}

void num_args_client(int argc)
{
    if(argc != 4)
    {
        printf("usage: cclient handle server-name server-port\n");
        printf("handle:     \t is this clients handle(name), max 100 characters\n");
        printf("server-name:\t is the remote machine running the server\n");
        printf("server-port:\t is the port number of the server application\n");
        exit(0);
    }
}

void check_handle_name(char * name)
{
    int len = strlen(name);
    if(len > 101)
    {
        fprintf(stderr, "%s", "error handle name too long\n");
        exit(EXIT_FAILURE);
    }
}

void pdu_client_setup(command_line_struct* cmd, char *clients_handle, int sd)
{
    int handles = 0;
    uint8_t len = 0;
    uint16_t packet_len = 0;
    char* handle;
    char buf[MAXBUF];
    int index = 0;

    uint16_t total = htons(cmd->total_length);
    memcpy(buf, &total, 2); //lenght of packet

    index +=2;

    memcpy(&(buf[index]), &cmd->flag_command, 1); //flag of packet

    index++;
    uint8_t client_len = strlen(clients_handle); //handle lenght of sneding client
    memcpy(&(buf[index]), &client_len, 1); //length of client handle

    index++;

    memcpy(&(buf[index]), clients_handle, client_len); //storing handle name

    index+=client_len;

    memcpy(&(buf[index]), &(cmd->num_handles), 1); //storing number of handles
    index++;
    //buf = buf + 1;
    packet_len = HEADER+1+client_len+1;
    int increment = 0;
    while(handles < cmd->num_handles)
    {
       handle = get_handle(cmd, ++handles); 
       len = strlen(handle);
        index+=increment;
       memcpy(&(buf[index]), &len, 1);
       packet_len++;
       index++;

       memcpy(&(buf[index]), handle, len);
       packet_len+=len;
       increment = len;

    }
    index+=increment;
    memcpy(&(buf[index]), cmd->message, cmd->message_length+1);
    packet_len+=(cmd->message_length+1);
    uint16_t n_pack_len = htons(packet_len);

    memcpy(&(buf[0]), &n_pack_len, 2);

    int sent;
    sent = send(sd, buf, packet_len, 0);
    if(sent < 0)
    {
        perror("send call in pdu client setup");
        exit(-1);
    }


}

char * get_handle(command_line_struct *cmd, int handle)
{
    if(handle == 1)
    {
        return &(cmd->handle1[0]);
    }
    else if(handle == 2)
    {
        return &(cmd->handle2[0]);
    }
    else if(handle == 3)
    {
        return &(cmd->handle3[0]);
    }
    else if(handle == 4)
    {
        return &(cmd->handle4[0]);
    }
    else if(handle == 5)
    {
        return &(cmd->handle5[0]);
    }
    else if(handle == 6)
    {
        return &(cmd->handle6[0]);
    }
    else if(handle == 7)
    {
        return &(cmd->handle7[0]);
    }
    else if(handle == 8)
    {
        return &(cmd->handle8[0]);
    }
    else if(handle == 9)
    {
        return &(cmd->handle9[0]);
    }
    perror("in get_handle, handle number not between 1-9");
    return NULL;
}

void save_handle(command_line_struct *input, int number, char* handle)
{
    if(number == 1)
    {
        memcpy(input->handle1, handle, strlen(handle)+1);
    }
    else if(number == 2)
    {
        memcpy(input->handle2, handle, strlen(handle)+1);
    }
    else if(number == 3)
    {
        memcpy(input->handle3, handle, strlen(handle)+1);
    }
    else if(number == 4)
    {
        memcpy(input->handle4, handle, strlen(handle)+1);
    }
    else if(number == 5)
    {
        memcpy(input->handle5, handle, strlen(handle)+1);
    }
    else if(number == 6)
    {
        memcpy(input->handle6, handle, strlen(handle)+1);
    }
    else if(number == 7)
    {
        memcpy(input->handle7, handle, strlen(handle)+1);
    }
    else if(number == 8)
    {
        memcpy(input->handle8, handle, strlen(handle)+1);
    }
    else if(number == 9)
    {
        memcpy(input->handle9, handle, strlen(handle)+1);
    }
    else
    {
        printf("your amound of handles :%d it too large\n", number);
        perror("cant handle more than 9 handle names, error in save_handle");
        exit(-1);
    }
}

void save_source_handle(command_line_struct *input, char* handle)
{
    memcpy(input->source_handle, handle, strlen(handle)+1);
}

void read_packet(command_line_struct *cmd, char* packet)
{
    uint8_t num_handles;
    uint8_t source_len;
    int index = 0;
    char name_buf[101];
    uint16_t len;
    memcpy(&(len), &(packet[0]), 2); //reading packet length
    cmd->total_length = ntohs(len); //storing packet in struct in host order
    index+=2; //read lenght increment by 2
    memcpy(&(cmd->flag_command), &(packet[index]), 1); //store flag in struct
    index++; //read flag increment by 1
    memcpy(&source_len, &(packet[index]), 1); //store source len
    memcpy(&(cmd->source_len), &(packet[index]), 1); //store source len in struct
    index++; //read len of source handle increment by 1
    add_null(name_buf, &(packet[index]), source_len); //adding null to source handel

    save_source_handle(cmd, name_buf);
    memcpy(cmd->source_handle, name_buf, source_len+1);
    index+=source_len;
    memcpy(&(cmd->num_handles), packet+index, 1);
    index++;
    num_handles = cmd->num_handles;
    

    uint8_t dest_len;
    while(num_handles > 0)
    {
        memcpy(&dest_len, packet+index, 1);

        index++; 
        add_null(name_buf, packet+index, dest_len);
        save_handle(cmd, num_handles, name_buf);
        num_handles--;
        index+=dest_len;

        
    }
    uint16_t left_over = cmd->total_length - index;
    
    memcpy(cmd->message, packet+index, left_over);
}
void add_null(char* buf, char *s, int len)
{
    
    memcpy(buf, s, len);
    buf[len] = '\0';
}

void send_flag4(command_line_struct *cmd, char* handle, int socket_number)
{
    char buf[MAXBUF];
    int sent;
    char flag = 4;
    uint16_t mess_len = strlen(cmd->in_buf);
    int times = 0;
    if(mess_len > 200)
    {
        times = (mess_len / 200) + 1;
        /*need to break it up */
    }
    uint8_t client_len = strlen(handle);
    uint16_t tot = HEADER+1+client_len+mess_len+1;
    tot = htons(tot);
    memcpy(&(cmd->source_handle), handle, strlen(handle));
    memcpy(&(buf[0]),&tot, 2);
    memcpy(&(buf[2]),&flag, 1);
    memcpy(&(buf[3]), &client_len, 1);
    memcpy(&(buf[4]), &(handle[0]), client_len);
    memcpy(&(buf[4+client_len]), cmd->in_buf, mess_len+1);
    sent = send(socket_number, buf, ntohs(tot), 0);
    if(sent < 0)
    {
        perror("error in sending packet with flag 4");
        exit(-1);
    }
   
}

void send_flag7(command_line_struct *cmd, int socket_number, int handle_num)
{
    int sent;
    char buf[HEADER+1+MAX_HANDLE];
    uint16_t tot_len;
    uint8_t flag = FLAG_7;
    uint8_t handle_len;
    char* handle = get_handle(cmd, handle_num);
    handle_len = strlen(handle);
    tot_len = HEADER+1+handle_len;
    tot_len = htons(tot_len);
    if(socket_number == -1)
    {
        perror("in send_flag, socket number is -1");
        exit(-1);
    }
    memcpy(buf, &tot_len, 2);
    memcpy(&(buf[2]), &flag, 1);
    memcpy(&(buf[3]), &handle_len, 1);
    memcpy(&(buf[4]), handle, handle_len);
    sent = send(socket_number, buf, ntohs(tot_len), 0);
    if(sent < 0)
    {
        perror("sending flag 7");
        exit(-1);
    }
   
}

void send_packet11(int tot, int sd)
{
    int sent;
    char flag = FLAG_11;
    uint16_t len = htons(HEADER + 4);
    char buf[HEADER+4];
    uint32_t count = htonl(tot);
    memcpy(&(buf[0]), &len, 2);
    memcpy(&(buf[2]), &flag, 2);
    memcpy(&(buf[HEADER]), &count, 4);
    sent = send(sd, buf, ntohs(len), 0);
    if(sent < 0)
    {
        perror("error in sending packet 11");
        exit(-1);
    }
    
}

void send_packet12(char *handle_name, int sd)
{
    int sent;
    int flag = FLAG_12;
    char buf[MAX_HANDLE+HEADER+1];
    uint8_t handle_len = strlen(handle_name);
    uint16_t len = handle_len + HEADER + 1;
    len = htons(len);
    memcpy(&(buf[0]), &len, 2);
    memcpy(&(buf[2]), &flag, 1);
    memcpy(&(buf[HEADER]), &handle_len, 1);
    memcpy(&(buf[HEADER+1]), &(handle_name[0]), handle_len);
    sent = send(sd, buf, ntohs(len), 0);
    if(sent < 0)
    {
        perror("send: in sending packet 12");
        exit(-1);
    }

}

void read_packet4(char* buf, uint16_t len, char flag)
{
    char mess_buf[MAXBUF];
    char handlebuf[MAX_HANDLE];
    uint8_t handle_len;
    len = ntohs(len);
    memcpy(&handle_len, &(buf[HEADER]), 1);
    memcpy(handlebuf, &(buf[HEADER+1]), handle_len);
    handlebuf[handle_len] = '\0';
    memcpy(mess_buf, &(buf[HEADER+1+handle_len]), len-(HEADER+1+handle_len));
    printf("%s: %s\n", handlebuf, mess_buf);

}

void save_packet4(char* buf, uint16_t len, char flag, char **mess_buf, char** handlebuf)
{
    uint8_t handle_len;
    len = ntohs(len);
    memcpy(&handle_len, &(buf[HEADER]), 1);
    memcpy(*(handlebuf), &(buf[HEADER+1]), handle_len);
    handlebuf[handle_len] = '\0';
    memcpy(*(mess_buf), &(buf[HEADER+1+handle_len]), len-(HEADER+1+handle_len));

}

void read_packet7(char * buf, uint16_t len, char flag)
{
    char handlebuf[MAX_HANDLE];
    uint8_t handle_len;
    len = ntohs(len);
    memcpy(&handle_len, &(buf[HEADER]), 1);
    memcpy(handlebuf, &(buf[HEADER+1]), handle_len);
    handlebuf[handle_len] = '\0';
    printf("client with handle  %s does not exits\n", handlebuf);
    
}

int read_packet11(char* buf, uint16_t len, char flag)
{
    uint32_t numb_handles;
    memcpy(&numb_handles, &(buf[HEADER]), 4);
    return ntohl(numb_handles);
}

void read_packet12(char* buf, uint16_t len, char flag)
{
    char handle_buf[MAX_HANDLE];
    uint8_t handle_length;
    memcpy(&handle_length, &(buf[HEADER]), 1);
    memcpy(handle_buf, &(buf[HEADER+1]), handle_length);
    handle_buf[handle_length] = '\0';
    printf("handle name: %s\n", handle_buf);
    
}

void read_packet13(char* buf, uint16_t len, char flag)
{

}
void print_cmd_struct(command_line_struct * cmd)
{
        printf("total length: %d\t, flag_command: %d\n", cmd->total_length, cmd->flag_command);
        printf("number of handles:%d\t message length: %d\n", cmd->num_handles, cmd->message_length);
    if(cmd->num_handles == 1)
    {
        printf("handle1: %s\n", cmd->handle1);
    }
    else if(cmd->num_handles == 2)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
    }
    else if(cmd->num_handles == 3)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
    }
    else if(cmd->num_handles == 4)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
        printf("handle4: %s\n", cmd->handle4);
    }
    else if(cmd->num_handles == 5)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
        printf("handle4: %s\n", cmd->handle4);
        printf("handle5: %s\n", cmd->handle5);
    }
    else if(cmd->num_handles == 6)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
        printf("handle4: %s\n", cmd->handle4);
        printf("handle5: %s\n", cmd->handle5);
        printf("handle6: %s\n", cmd->handle6);
    }
    else if(cmd->num_handles == 7)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
        printf("handle4: %s\n", cmd->handle4);
        printf("handle5: %s\n", cmd->handle5);
        printf("handle6: %s\n", cmd->handle6);
        printf("handle7: %s\n", cmd->handle7);
    }
    else if(cmd->num_handles == 8)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
        printf("handle4: %s\n", cmd->handle4);
        printf("handle5: %s\n", cmd->handle5);
        printf("handle6: %s\n", cmd->handle6);
        printf("handle7: %s\n", cmd->handle7);
        printf("handle8: %s\n", cmd->handle8);
    }
    else if(cmd->num_handles == 9)
    {
        printf("handle1: %s\n", cmd->handle1);
        printf("handle2: %s\n", cmd->handle2);
        printf("handle3: %s\n", cmd->handle3);
        printf("handle4: %s\n", cmd->handle4);
        printf("handle5: %s\n", cmd->handle5);
        printf("handle6: %s\n", cmd->handle6);
        printf("handle7: %s\n", cmd->handle7);
        printf("handle8: %s\n", cmd->handle8);
        printf("handle9: %s\n", cmd->handle9);
    }

    printf("message: %s\n", cmd->message);

}

