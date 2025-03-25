#include <l_hash.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cutils.h>

unsigned long l_string_hash(char *str){
   unsigned long hash = 5381;
   int c;

   while((c = *str++))
      hash = ((hash << 5) + hash) + c;

   return hash;
}

void delete_hash_table(L_HASH_TABLE *table){
   for(int i = 0; i < MAX_HASH_TABLE_SIZE; ++i){
      L_HASH_NODE *curr = table->table[i];
      L_HASH_NODE *next;
      while(curr){
         if(curr->key_val.type == STRING){
            free(curr->key_val.str);
         }
         next = curr->next;
         free(curr->key_val.key);
         free(curr);
         curr = next;
      }
   }
   // free(table);
}

int l_contains(L_HASH_TABLE *table, char *key){
   unsigned long hash = l_string_hash(key);
   int index = hash % MAX_HASH_TABLE_SIZE;
   L_HASH_NODE *curr = table->table[index];
   int found = 0;

   while(curr){
      if(strcpy(curr->key_val.key, key) == 0){
         found = 1;
         break;
      }
      curr = curr->next;
   }
   return found;
}

void l_get_info(L_HASH_TABLE *table, char *key, L_HASH_NODE **node, unsigned long *hash, int *index){
   *hash = l_string_hash(key);
   *index = *hash % MAX_HASH_TABLE_SIZE;
   *node = NULL;
  
   L_HASH_NODE *curr = table->table[*index];
   
   while(curr){
      if(strcmp(key, curr->key_val.key) == 0){
         *node = curr;
         break;
      }
      curr = curr->next;
   }
}

L_KEY_VAL *l_put_number(L_HASH_TABLE *table, char *key, double value){
   unsigned long hash;
   L_HASH_NODE *curr;
   int index;

   l_get_info(table, key, &curr, &hash, &index);
   
   if(curr){
      if(curr->key_val.type == STRING){
         free(curr->key_val.str);
         curr->key_val.type = NUMBER;
      }
      curr->key_val.number = value;
      return &curr->key_val;
   }

   L_HASH_NODE *new_node = assert_alloc(sizeof(L_HASH_NODE));
   new_node->key_val.type = NUMBER;
   new_node->key_val.key = strdup(key);
   new_node->key_val.number = value;

   curr = table->table[index];

   if(curr == NULL){
      table->table[index] = new_node;
   }else{
      while(curr->next){
         curr = curr->next;
      }

      new_node->next = curr->next;
      curr->next = new_node;
   }

   return &new_node->key_val;
}

L_KEY_VAL *l_put_string(L_HASH_TABLE *table, char *key, char *value){
   unsigned long hash;
   L_HASH_NODE *curr;
   int index;

   l_get_info(table, key, &curr, &hash, &index);
   
   if(curr){
      if(curr->key_val.type == STRING){
         free(curr->key_val.str);
      }
      curr->key_val.type = STRING;
      curr->key_val.str = strdup(value);
      return &curr->key_val;
   }

   L_HASH_NODE *new_node = assert_alloc(sizeof(L_HASH_NODE));
   new_node->key_val.type = STRING;
   new_node->key_val.key = strdup(key);
   new_node->key_val.str = strdup(value);

   curr = table->table[index];

   if(curr == NULL){
      table->table[index] = new_node;
   }else{
      while(curr->next){
         curr = curr->next;
      }

      new_node->next = curr->next;
      curr->next = new_node;
   }

   return &new_node->key_val;
}

L_KEY_VAL *l_get_value(L_HASH_TABLE *table, char *key){
   unsigned long hash = l_string_hash(key);
   int index = hash % MAX_HASH_TABLE_SIZE;
  
   L_HASH_NODE *curr = table->table[index];
   
   while(curr){
      if(strcmp(key, curr->key_val.key) == 0){
         break;
      }
      curr = curr->next;
   }

   if(curr == NULL) return NULL;

   return &curr->key_val;
}
/*
int main(){
   printf("Testing hash table\n");
   L_HASH_TABLE table = {0};

   l_put_string(&table, "hello", "Hey there obiwan");
   l_put_string(&table, "hello1", "Hey there obiwan");
   l_put_number(&table, "hello2", 10);

   L_KEY_VAL *key_val = l_get_value(&table, "hello");
   printf("hello: %s\n", key_val->str);

   L_KEY_VAL *key_val1 = l_get_value(&table, "hello1");
   printf("hello1: %s\n", key_val1->str);

   L_KEY_VAL *key_val2 = l_get_value(&table, "hello2");
   printf("hello2: %f\n", key_val2->number);

   l_put_number(&table, "hello", 11);

   key_val = l_get_value(&table, "hello");
   printf("hello: %f\n", key_val->number);

   delete_hash_table(&table);
   return 0;
} */
