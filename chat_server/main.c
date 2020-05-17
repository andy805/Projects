#include "table.h"
#include <stdio.h>

int main(int argc, char * argv[])
{
    hashtable* my_hash_table = new_hash_table();
    insert(my_hash_table, "andy", 10);
    insert(my_hash_table, "alyah", 100);
    insert(my_hash_table, "marvin",1000);
    printf("count of table: %d\n", my_hash_table->count);
    printf("size of table: %d\n", my_hash_table->size);
    printf("andys number: %d\n", search(my_hash_table, "andy"));
    printf("alyahs number: %d\n", search(my_hash_table, "alyah"));
    printf("marvins number: %d\n", search(my_hash_table, "marvin"));

    int flag = ht_remove_client(my_hash_table, "andy");
    printf("flag for removing client: %d\n", flag);
    int search_flag = search(my_hash_table, "andy");
    printf("flag for searching client: %d\n", search_flag);

    int flag2 = ht_remove_client(my_hash_table, "alyah");
    printf("flag for removing client: %d\n", flag2);
    int search_flag2 = search(my_hash_table, "alyah");
    printf("flag for searching client: %d\n", search_flag2);

    int flag3 = ht_remove_client(my_hash_table, "marvin");
    printf("flag for removing client: %d\n", flag3);
    int search_flag3 = search(my_hash_table, "marvin");
    printf("flag for searching client: %d\n", search_flag3);

    delete_hash_table(my_hash_table);
    return 0;
}
