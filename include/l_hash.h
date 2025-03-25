#pragma once

#define MAX_HASH_TABLE_SIZE 13

typedef enum l_type{
   NUMBER,
   STRING,
} L_TYPE;

typedef struct l_key_val{
   char *key;
   L_TYPE type;
   union{
      double number;
      // void *data;
      char *str;
   };
} L_KEY_VAL;

typedef struct l_hash_node{
   struct l_hash_node *next;
   L_KEY_VAL key_val;
} L_HASH_NODE;

typedef struct l_hash_table{
   L_HASH_NODE *table[MAX_HASH_TABLE_SIZE];
   int size;
} L_HASH_TABLE;

unsigned long l_string_hash(char *str);

void delete_hash_table(L_HASH_TABLE *table);
int l_contains(L_HASH_TABLE *table, char *key);
L_KEY_VAL *l_put_number(L_HASH_TABLE *table, char *key, double value);
L_KEY_VAL *l_put_string(L_HASH_TABLE *table, char *key, char *value);

L_KEY_VAL *l_get_value(L_HASH_TABLE *table, char *key);
