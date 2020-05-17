
typedef struct client_node
{
    char * handle_name;
    int client_socket;
    int valid_flag;
}client_node;

typedef struct hashtable
{
    int count;
    int size;
    client_node** clients;
    int* sockets;
}hashtable;

hashtable * new_hash_table();

client_node * new_client(char * handle_name, int socket_number);

void delete_client(client_node * p);

void delete_hash_table(hashtable * table);

int hash(char * handle_name, int prime_number, int size);

int get_hash(char * handle_name, int size, int attempts);

void insert(hashtable *ht, char * handle_name, int socket_number);

int search(hashtable *ht, char * handle_name);

int ht_remove_client(hashtable * ht, char* handle_name);

void insert_socket_array(int socket_number, hashtable *ht);

hashtable* ht_new_size(int base_size, int num);

void resize(hashtable* ht, int base_size);
