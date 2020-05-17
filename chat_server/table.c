#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "table.h"

#define SIZE 750
#define FACTOR 3
#define PRIME_1 307
#define PRIME_2 197

client_node DELETED_CLIENT = {NULL, -1, 0};


hashtable * new_hash_table()
{
    hashtable * ht = (hashtable *)malloc(sizeof(hashtable));
    ht->size = SIZE;
    ht->count = 0;
    ht->clients = calloc((size_t)ht->size, sizeof(client_node*)); //init 500 pointers that
                                                                 //will point to my client nodes

    ht->sockets = (int*)malloc(ht->size *  sizeof(int));
    return ht;
}

/*  initializing function for a client to add to hash table
 *  key = handle_name
 *  value = socket number 
 *  return a pointer to an initialized client */
client_node* new_client(char* handle_name, int socket_number)
{
    client_node* client = (client_node*)malloc(sizeof(client_node));
    client->handle_name = strdup(handle_name);
    client->client_socket = socket_number;
    client->valid_flag = 1;
    return client;
} 

void delete_client(client_node * p)
{
    free(p->handle_name);
    free(p);
}

void delete_hash_table(hashtable * table)
{
    int i;
    for(i = 0; i < table->size; i++)
    {
        client_node * client = table->clients[i];
        if(client != NULL)
        {
            if(client != &DELETED_CLIENT)
            {
                delete_client(client);
            }
        }
    }
    free(table->clients);
    free(table->sockets);
    free(table);
}

/* handle_name = the clients handle name
 * prime_number = a prime number for this hashing
 * size = the size of the hash table, to get my indices
 * outputs an index for to index into hash table 
 * note using 307 for prime number */
int hash(char * handle_name, int prime_number, int size)
{
    int i;
    long hash = 0;
    int len = strlen(handle_name);
    for(i = 0; i < len; i++)
    {
        hash += (long)pow(prime_number, len - (i+1)) * handle_name[i];
        hash = hash % size;
    }
    return (int)hash;
}
/*handling collision by double hashing 
 * attemps = how many collisions have happened when looking for an empty spot in table
 * size = size of our hash table
 */
int get_hash(char * handle_name, int size, int attempts)
{
    int first_hash = hash(handle_name, PRIME_1, size);
    int second_hash = hash(handle_name, PRIME_2, size);
    int hash = first_hash + (attempts * (second_hash + 1)) ;
    return hash % size;

}

void insert(hashtable *ht, char * handle_name, int socket_number)
{
    int load = ht->count * 100 / ht->size;
    if(load > 60)
    {
        resize(ht, FACTOR);
    }
    int i = 0;
    int index = get_hash(handle_name, ht->size, i);
    client_node* client = new_client(handle_name, socket_number);
    client_node* curr_client = ht->clients[index];
    insert_socket_array(socket_number, ht);
    while(curr_client != NULL)
    {
        if(curr_client == &DELETED_CLIENT)
        {
            ht->clients[index] = client;
            ht->count++;
            return;
        }
        index = get_hash(handle_name, ht->size, ++i);
        curr_client = ht->clients[index];
    }
    ht->clients[index] = client;
    //printf("in insert, name: %s\n", ht->clients[index]->handle_name);
    ht->count++;
}

/*note might need to modify to return the client node instead of 
 * just the socket number */
int search(hashtable *ht, char * handle_name)
{
    int i = 0;
    int index = get_hash(handle_name, ht->size, i);
    client_node* curr_client = ht->clients[index];
    //printf("search, before while, name: %s\n", curr_client->handle_name);
    while(curr_client != NULL && curr_client != &DELETED_CLIENT)
    { 
        if(0 == strcmp(curr_client->handle_name, handle_name))
        {
            /* i have found my client */
            return curr_client->client_socket;
            /*to return pointer to node
             * and dont forget to change function return type
             * return curr_client;
             */
        }
        index = get_hash(handle_name, ht->size, ++i);
        curr_client = ht->clients[index];
    }
    return -1;
}

/* ht = hash table
 * handle_name = key, the key into the hash table 
 * outputs 1 on success, 0 on failure */
int ht_remove_client(hashtable* ht, char* handle_name)
{
    int i = 0;
    int index = get_hash(handle_name, ht->size, i);
    client_node *curr_client = ht->clients[index];
    while(curr_client != NULL)
    {
       if(curr_client != &DELETED_CLIENT)
       {
           if(0 == strcmp(curr_client->handle_name, handle_name))
           {
               ht->sockets[curr_client->client_socket] = 0;
               delete_client(curr_client);
               ht->clients[index] = &DELETED_CLIENT;
               ht->count--;
               return 1;
           }
       }
    }
    return 0;

}

void insert_socket_array(int socket_number, hashtable *ht)
{
    int i = 0;
    while(ht->sockets[i] != 0)
    {
        i++;
    }
    ht->sockets[i] = socket_number;
//    printf("sockets[%d] = %d\n", i, socket_number);
}

hashtable* ht_new_size(int base_size, int num)
{
    hashtable * ht = (hashtable *)malloc(sizeof(hashtable));
    ht->size = base_size * num;
    ht->count = 0;
    ht->clients = calloc((size_t)ht->size, sizeof(client_node*)); //init 500 pointers that
                                                                 //will point to my client nodes

    ht->sockets = (int*)malloc(ht->size *  sizeof(int));
    return ht;
    
}

void resize(hashtable* ht, int base_size)
{
    int i;
    hashtable* new_table = ht_new_size(ht->size, base_size);
    for( i = 0; i < new_table->size; i++)
    {
        client_node* client = ht->clients[i];
        if(client != NULL && client != &DELETED_CLIENT)
        {
            insert(new_table, client->handle_name, client->client_socket);
        }
    }

    ht->size = new_table->size;
    ht->count = new_table->count;

    client_node** tmp_clients = ht->clients;
    ht->clients = new_table->clients;
    new_table->clients = tmp_clients;

    delete_hash_table(new_table);

}
