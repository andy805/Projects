/******************************************************************************
* myClient.c
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "helper_client.h"

#define DEBUG_FLAG 1

void  sendToServer(int socketNum, char* message, command_line_struct* input, char *client_handle);
void checkArgs(int argc, char * argv[]);
int exit_flag = 0;
int list_flag = 0;

int main(int argc, char * argv[])
{
	int socketNum = 0;  
	char sendBuf[MAXBUF];
    char* handle_name = NULL;
    char* server_name = NULL;
    int server_port = -1;
    command_line_struct input;
    parse_command_line(argc, argv, &handle_name, &server_name, &server_port);
	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);
    set_up_server(socketNum, argv[1]);
    while(1)
    {    
	    sendToServer(socketNum, sendBuf, &input, argv[1]);
    }
	close(socketNum);
	return 0;
}

void set_up_server(int socket_number, char* handle_name)
{
    char sendBuf[MAX_HANDLE+HEADER+1];
    uint8_t handle_len = strlen(handle_name);
    uint16_t packet_len = HEADER + handle_len + 1 + 1;
    uint8_t flag = FLAG_1;
    int ready;
    int sent = 0;
    if(handle_len > 100)
    {
        perror("handle name too long, max 100 characters");
        exit(-1);
    }
    packet_len = htons(packet_len);
    memcpy(sendBuf, &(packet_len), 2);
    memcpy(sendBuf+2, &flag, 1);
    memcpy(sendBuf+3, &handle_len, 1);
    memcpy(sendBuf+4, handle_name, handle_len+1);
    sent = send(socket_number, sendBuf, ntohs(packet_len), 0);
    if(sent < 0)
    {
        perror("send call");
        exit(-1);
    }

        printf("$: ");
        fflush(stdout);
    ready =  selectCall(socket_number, 1, 0, TIME_IS_NULL);
    if(ready < 0)
    {
        perror("receive call in set up server");
        exit(-1);
    }
    char buf[MAXBUF];
    int receive_len = 0;
    uint8_t receive_flag;
    uint16_t receive_length;
    if((receive_len = recv(socket_number, &receive_length, 2, MSG_WAITALL)) < 0)
    {
        perror("recieve call, in settting up server connection");
        exit(-1);
    }
    if((receive_len = recv(socket_number, &(buf[2]), ntohs(receive_length)-2, MSG_WAITALL)) < 0)
    {
        perror("recieve call, in settting up server connection");
        exit(-1);
    }
    memcpy(&(buf[0]), &receive_length, 2);
    //memcpy(&receive_flag, buf+2, 1);
    receive_flag = buf[2];
    if(receive_flag == 3)
    {
        perror("not unique name");
        exit(-1);
    }
    
}

void sendToServer(int socketNum, char* sendBuf, command_line_struct* input, char *client_handle)
{
    fd_set fileD_set;
    FD_ZERO(&fileD_set);
    FD_SET(socketNum, &fileD_set);
    FD_SET(0, &fileD_set);
    int num_ready = 0;
    if((num_ready = select(socketNum+1, &fileD_set, NULL, NULL, NULL)) < 0)
    {
        perror("select in client, sendToServer, first call");
        exit(-1);
    }
        //printf("$: ");
        //fflush(stdout);
    if(FD_ISSET(0, &fileD_set))
    {
        //printf("$: ");
        //fflush(stdout);
        process_input(input, sendBuf);
        if(input->flag_command == FLAG_5)
        {
            /* %m message packet, dont block waiting for responce from server */
            pdu_client_setup(input, client_handle, socketNum);

            /*send*/
        }
        else if(input->flag_command == FLAG_4)
        {
            /* %b broadcast */
            send_flag4(input, client_handle, socketNum);
            printf("\n$: ");
            fflush(stdout);
        }
        else if(input->flag_command == FLAG_10)
        {
            /* %l requesting the list of handles
             * must continue to process any other message that come in until flag 11 comes in */
            send_header(FLAG_10, socketNum);
        }
        else if(input->flag_command == FLAG_8)
        {
            /* this is %e, exit, cannot exit until flag 9 recieved from server 
             * so keep processing messages, do not block until flag 9 recieved from server */
            send_header(FLAG_8, socketNum);
            exit_flag = 1;
            return;
        }
        else
        {
            perror("wrong command");
            return;
        }

    }
    else if(FD_ISSET(socketNum, &fileD_set))
    {
        int message_len = 0;
        command_line_struct cmd;
        char read_buf[MAXBUF];
        uint16_t len_buf;
        uint8_t flag_read;
        if((message_len = recv(socketNum, &len_buf, 2, MSG_WAITALL)) < 0)
        {
            perror("error in recv call, reading from server");
            exit(-1);
        }
        if(message_len == 0)
        {
            perror("error connecting to server");
            exit(-1);
        }
        if((message_len = recv(socketNum, &read_buf[2], ntohs(len_buf) - 2, MSG_WAITALL)) < 0)
        {
            perror("error in recv call, reading from server");
            exit(-1);
        }

        memcpy(&(read_buf[0]), &len_buf, 2);
        memcpy(&flag_read, &(read_buf[2]), 1);
        if(flag_read == FLAG_4)
        {
            
            read_packet4(read_buf, len_buf, flag_read);
            printf("$: ");
            fflush(stdout);
        }
        else if(flag_read == FLAG_5)
        {
            read_packet(&cmd, read_buf);
            //printf("\n messege len: %d\n", strlen(cmd.message));
            printf("%s: %s\n", cmd.source_handle, cmd.message);
            printf("\n$: ");
            fflush(stdout);
            
        }
        else if(flag_read == FLAG_7)
        {
            read_packet7(read_buf, len_buf, flag_read);
            //printf("client with handle %s does not exist\n", cmd.message);
            printf("$: ");
            fflush(stdout);
        }
        else if(flag_read == FLAG_9)
        {
            if(exit_flag == 1)
            {
                printf("exiting\n");
                close(socketNum);
                exit(0);
            }
        }
        else if(flag_read == FLAG_11)
        {
            uint32_t numb_handles = read_packet11(read_buf, len_buf, flag_read);
            printf("number of handles: %d\n", numb_handles);
            list_flag = 1;
        }
        else if(flag_read == FLAG_12)
        {
            if(list_flag == 1)
            {
                read_packet12(read_buf, len_buf, flag_read);
            }
        }
        else if(flag_read == FLAG_13)
        {
            list_flag = 0;
            //read_packet13(read_buf, len_buf, flag_read);
            printf("$: ");
            fflush(stdout);
            return;
        }
    }
}

void addPDU(uint16_t *pdu_length, char* packet, char *data, int data_length)
{
    memcpy((void*)packet, (void *)pdu_length, 2);
    memcpy((packet)+2, data, data_length);

}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 3)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}
}

int command_read(char * command)
{
    if(0 == strcmp(command, "%M") || 0 == strcmp(command, "%m"))
    {
        return FLAG_5;
    }
    else if( 0 == strcmp(command, "%b"))
    {
        return FLAG_4;
    }
    else if(0 == strcmp(command, "%L") || 0 == strcmp(command, "%l"))
    {
        return FLAG_10;
    }
    else if(0 == strcmp(command, "%E") || 0 == strcmp(command, "%e"))
    {
        return FLAG_8;
    }

    else if(command[0] == 37 && command[1] == 66)
    {
        printf("here\n");
        return FLAG_4;
    }
    else
    {
        return -1;
    }
}

void process_input(command_line_struct* input, char *sendBuf)
{
	char aChar = 0;
	int sendLen = 0;        //amount of data to send
    int flag = -1;
    int num_handles = -1;
    int argc = 0;
    
    //uint8_t input_flag;
	// Important you don't input more characters than you have space 
        while (sendLen < (MAXBUF - 1) && aChar != '\n')
        {
            aChar = getchar();
            if (aChar != '\n')
            {
                sendBuf[sendLen] = aChar;
                sendLen++;
            }
        }
        if(sendLen > (MAXBUF -1))
        {
            perror("input length too long\n");
            exit(-1);
        }
        sendBuf[sendLen] = '\0';
        sendLen++;  //we are going to send the null
        
        input->total_length = sendLen;
        if(sendLen > 1400)
        {
            perror("error input too long\n");
            return;
        }
        char* cpybuf = strdup(sendBuf);
        char* split = strtok(cpybuf, " ");
        while(split != NULL)
        {
            if(flag == -1)
            {
                input->flag_command = command_read(split);
                flag = input->flag_command;
                if(FLAG_4 == flag)
                {
                    break;
                }
          
            }
            else 
            {
                if(num_handles == -1 && flag != FLAG_4)
                {
                    num_handles = atoi(split);
                    if(num_handles > 9 || num_handles < 1)
                    {
                        perror("invalid number of handles");
                        return;
                    }
                    input->num_handles = num_handles;
                }
                else
                {
                    if(num_handles > 0)
                    {
                        save_handle(input, num_handles, split);
                        num_handles--;

                    }
                    else
                    {
                        break;
                    }
                }
            }
            argc++;
            split = strtok(NULL, " ");
        }
        if(input->flag_command == FLAG_8)
        {
            return;
        }
        if(input->flag_command == FLAG_10)
        {
            return;
        }
        if(argc < 3 && input->flag_command != 4)
        {
            perror("bad input. ex %m 1 handle1 txt message");
            exit(-1);
        }
        if(input->flag_command == FLAG_4)
        {
            save_text_only(sendBuf, input);
            //printf("check b: %s\n", input->in_buf);    
        }
        char* pointer = NULL;
        save_message(sendBuf, input, &pointer);
        input->message_length = strlen(pointer);
        memcpy(input->message, pointer, input->message_length+1);


}



void save_message(char* buf, command_line_struct* cmd, char** pointer)
{
    int num_handles = cmd->num_handles + 2;
    
    int index = 0;
    while(num_handles > 0)
    {
        if(buf[index] != ' ' && buf[index+1] == ' ')
        {
            num_handles--;
        }
        if(buf[index] != ' ' && buf[index+1] == '\0')
        {
            num_handles--;
        }
        index++;
    }
    if(buf[index] == '\0')
    {
        *pointer = &buf[index];
        return;
    }
    while(buf[index] != ' ')
    {
        index++;
    }
    *pointer = &buf[++index];

}

void save_text_only(char* buf, command_line_struct *cmd)
{
    int i = 0;
    char c = '\0';
    while(buf[i] != 'b' && buf[i] != 'B')
    {
        i++;
    }
    i++;
    if(buf[i] == '\0')
    {
        memcpy(&(cmd->in_buf[0]), &(c), 1); //getting null character too
    }
    int len = strlen(buf);
    len = len-i;
    memcpy(&(cmd->in_buf[0]), &(buf[i]), len+1); //getting null character too
}
