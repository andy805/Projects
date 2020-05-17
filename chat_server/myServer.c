
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
#include "gethostbyname6.h"
#include "table.h"
#include "helper_client.h"

#define DEBUG_FLAG 1
#define MAX_CLIENTS 2000
#define HEADER 3
#define BUF_HANDLE 102

int recvFromClient(int clientSocket, char* buf);
int checkArgs(int argc, char *argv[]);
int selectServer(int server_socket);
void handle_select(int server_socket, int client_socket[],int *max_socket_num, hashtable* ht);
void handle_packet(hashtable* ht, char* packet, int sd);
//void add_null(char* buf, char *s, int len);

int main(int argc, char *argv[])
{
	int server_socket = 0;   //socket descriptor for the server socket
	int portNumber = 0;
    int client_sockets[MAX_CLIENTS];
    int max_socket_num = 0;
    hashtable* my_table = new_hash_table();
	portNumber = checkArgs(argc, argv);
	//create the server socket
	server_socket = tcpServerSetup(portNumber);

	while(1)
    {
        handle_select(server_socket, client_sockets, &max_socket_num,  my_table);
    }
	close(server_socket);
	return 0;
}

int recvFromClient(int clientSocket, char* buf)
{
    uint16_t header_length = 0;
	//char buf[MAXBUF];
	int messageLen = 0;
	int messageLen2 = 0;
        // Use a time value of 1 second (so time is not null)
        while (selectCall(clientSocket, 1, 0, TIME_IS_NOT_NULL) == 0)
        {
            printf("Select timed out waiting for client to send data\n");
        }
        
        //now get the data from the client_socket (message includes null)
        if ((messageLen = recv(clientSocket, &header_length, 2, MSG_WAITALL)) < 0)
        {
            perror("recv call");
            exit(-1);
        }
        header_length = ntohs(header_length);

        if ((messageLen2 = recv(clientSocket, buf, header_length-2, MSG_WAITALL)) < 0)
        {
            perror("recv call");
            exit(-1);
        }
        

        if(header_length == 0)
        {
            perror("client not sending data\n");
            return 0;
        }
        return 1;
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

void handle_select(int server_socket, int client_sockets[],int *max_socket_num, hashtable* ht)
{
    fd_set fileD_set;
    FD_ZERO(&fileD_set);
    FD_SET(server_socket, &fileD_set);
    *max_socket_num = server_socket ;
    int max_sn = server_socket;
    int i, numReady,  sd;
    int client_socket = 0;
    struct sockaddr_in6 client_info;
    int client_info_size = sizeof(client_info);
    char handle_buf[MAXBUF];
    char read_buf[MAXBUF];
    uint16_t init_length;
    for(i = 0; i < ht->count; i++)
    {
        sd = ht->sockets[i];
        if(sd > 0)
        {
            FD_SET(ht->sockets[i], &fileD_set);
        }
        if(sd > max_sn)
        {
            max_sn = sd;
        }
    }
    
    if((numReady = select(max_sn+1, &fileD_set, NULL, NULL, NULL)) < 0)
    {
        perror("select, in the handle_select function");
        exit(-1);
    }
    if(FD_ISSET(server_socket, &fileD_set))
    {
        /* this means that a client wants to connect so need to call accept*/
        if((client_socket = accept(server_socket, (struct sockaddr*) &client_info, (socklen_t*) &client_info_size)) < 0)
        {
            perror("error in accept call, in handle select function");
            exit(-1);
        }

        //printf("Client accepted.  Client IP: %s Client Port Number: %d\n",  
         //                   getIPAddressString(client_info.sin6_addr.s6_addr), ntohs(client_info.sin6_port));  
        int read = 0;
        /*receive clients sockets first packet */
        if((read = recv(client_socket, &init_length, 2, MSG_WAITALL)) < 0)
        {
            perror("recv call when trying to first connect with client, flag one check");
            exit(-1);
        }
        if(read == 0)
        {
            close(client_socket);
            return;
        }
        //printf("1. bytes received %d\n", read);
        //uint16_t length;
        //memcpy(&length, header_buf, 2);
        //length = ntohs(length);
        //printf("Lenght of incoming packet: %d\t this is when first connecting\n", length);

        if((read = recv(client_socket, &(handle_buf[2]), ntohs(init_length)-2, MSG_WAITALL)) < 0)
        {
            perror("in recv did not recieve enough data, when first connecting");
            exit(-1);
        }
        if((handle_buf[2]) != 1)
        {
            perror("client did not send flag one, exiting");
            exit(-1);
        }
        if(read == 0)
        {
            close(client_socket);
            return;
        }
        uint8_t client_len = 0;
        //init_length = ntohs(init_length);
        memcpy(handle_buf, &init_length, 2); /*saving len to my buf */
        memcpy(&client_len, &(handle_buf[3]), 1); /*saving the handle length to variable */
        char client_buf[101];  /*delcaringa buf to add null to hande name that i received*/
        add_null(client_buf, &(handle_buf[4]), client_len);/*adding null*/
        if(-1 == search(ht, client_buf))
        {
            /*handle name is not in the hash table which means its valid*/
            insert(ht, client_buf, client_socket);
            /*send back flag 2 */
            send_header(FLAG_2, client_socket);

        }
        else
        {
            /*found handle name, send back flag 3 */
            /*close client_socket*/
            send_header(FLAG_3, client_socket);
            close(client_socket);
        }

        
    }

    for(i = 0; i < ht->count; i++)
    {
        sd = ht->sockets[i];
        uint16_t packet_len = 0;
        if(FD_ISSET(sd, &fileD_set))
        {

            int message_len = recv(sd, &packet_len, 2, MSG_WAITALL);
            if(message_len < 0)
            {
                perror("error in receive. reading sockets header");
                exit(-1);

            }
            if(message_len == 0)
            {
                close(sd);
                for(i = 0; i < ht->count; i++)
                {
                    if(ht->sockets[i] == sd)
                    {
                        ht->sockets[i] = 0;
                        break;
                    }
                }
                for(i = 0; i < ht->size; i++)
                {
                    if(ht->clients[i] != NULL)
                    {
                        if(ht->clients[i]->client_socket == sd)
                        {
                            ht_remove_client(ht, ht->clients[i]->handle_name);
                            break;
                        }
                    }
                }
                return;
            }
            message_len = recv(sd, &(read_buf[2]), ntohs(packet_len)-2, MSG_WAITALL);

            memcpy(&(read_buf[0]), &packet_len, 2); /*copying len into buf */
            if(message_len < 0)
            {
                perror("error in receive. reading sockets non header");
                exit(-1);
            }
            else if(message_len == 0)
            {
                close(sd);
                for(i = 0; i < ht->count; i++)
                {
                    if(ht->sockets[i] == sd)
                    {
                        ht->sockets[i] = 0;
                        break;
                    }
                }
                for(i = 0; i < ht->size; i++)
                {
                    if(ht->clients[i] != NULL)
                    {
                        if(ht->clients[i]->client_socket == sd)
                        {
                            ht_remove_client(ht, ht->clients[i]->handle_name);
                            break;
                        }
                    }
                }
                return;
            }
            else
            {
                handle_packet(ht, read_buf, sd);
            }
            
        }
    }
}

void handle_packet(hashtable* ht, char* packet, int sd)
{
    int i;
    uint8_t flag;
    uint16_t packet_len;
    uint8_t num_handles;
    uint8_t source_len;
    int socket_number = 0;
    int sent = 0;
    command_line_struct cmd;
    int index = 0;
    memcpy(&packet_len, packet, 2);
    index+=2;
    memcpy(&flag, packet+index, 1);
    index++;
    memcpy(&source_len, packet+index, 1);
    index++;
    char buf[MAXBUF];
    memcpy(buf, packet, ntohs(packet_len));
    
    if(flag == FLAG_5)
    {
        read_packet(&cmd, packet); //changes packet 
        num_handles = cmd.num_handles;
        while(num_handles > 0)
        {
           socket_number = search(ht, get_handle(&cmd, num_handles));
           if(socket_number == -1)
           {
               /*handle name doesnt exit */
               send_flag7(&cmd, search(ht, cmd.source_handle), num_handles);           

           }
           else if(socket_number == 0)
           {
               perror("search from hash table returned 0");
               exit(-1);
           }
           else
           {
               /*handle name exists */
               
               sent = send(socket_number, buf, cmd.total_length, 0);
               if(sent < 0)
               {
                   perror("in handle_packets, send call in flag_5");
                   exit(-1);
               }
               
           }
           num_handles--;

        }
    }
    else if(flag == FLAG_4)
    {
        uint16_t temp_len;
        uint8_t temp_client_len;
        char mess_buf[MAXBUF];
        char handlebuf[MAX_HANDLE];
        memcpy(&temp_len, packet, 2);
        memcpy(&temp_client_len, packet+ HEADER, 1);
        memcpy(&(handlebuf[0]), packet+(HEADER+1), temp_client_len);
        handlebuf[temp_client_len] = '\0';
        memcpy(&(mess_buf[0]), packet+(HEADER+1+temp_client_len), ntohs(temp_len)-(HEADER+1+temp_client_len));
        memcpy(&(cmd.in_buf[0]), mess_buf, strlen(mess_buf)+1);
        
        memcpy(&(cmd.source_handle[0]), &(handlebuf[0]), strlen(handlebuf)+1);
        int numb_send = ht->count - 1;
        int table_index = 0;
        while(numb_send > 0)
        {
            if(ht->clients[table_index] != NULL && ht->clients[table_index]->handle_name != NULL)
            {
                if(0 != strcmp(ht->clients[table_index]->handle_name, handlebuf))
                {
                    send_flag4(&cmd, handlebuf,  ht->clients[table_index]->client_socket);
                    numb_send--;
                }
            }
            table_index++;
        }
        
    }

    else if(flag == FLAG_8)
    {
        send_header(FLAG_9, sd);
        close(sd);
        for(i = 0; i < ht->count; i++)
        {
            if(ht->sockets[i] == sd)
            {
                ht->sockets[i] = 0;
                break;
            }
        }
        
        for(i = 0; i < ht->size; i++)
        {
            if(ht->clients[i] != NULL)
            {
                if(ht->clients[i]->client_socket == sd)
                {
                    ht_remove_client(ht, ht->clients[i]->handle_name);
                    break;
                }
            }
        }
        return;
    }
    else if(flag == FLAG_10)
    {
        int numb_send = ht->count;
        int table_index = 0;
        send_packet11(ht->count, sd);
        while(numb_send > 0)
        {
            if(ht->clients[table_index] != NULL && ht->clients[table_index]->handle_name != NULL)
            {
                send_packet12(ht->clients[table_index]->handle_name, sd);
                numb_send--;
            }
            table_index++;
        }
        send_header(FLAG_13, sd);
        
    }


}


