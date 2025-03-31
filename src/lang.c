#include "lang.h"
#include <cutils/map.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 1024

void *assert_alloc(size_t size){
   void *var = calloc(1, size);
   if(var == NULL){
      perror("Malloc Fail\n"); exit(-1);
   }
   return var;
}

void l_token_free(L_TOKEN *token){
   if(token->type != VAL_NUM){
      free(token->str);
   }
}

int main(){
   L_TOKEN tokens[MAX_TOKENS];
   int index = 0;
   l_lexer_from_file("lang.l");

   while(!l_lexer_is_empty()){
      tokens[index] = l_read_token();
      if(tokens[index].type == NONE) break;
      printf("%s[%d:%d]\n", l_token_str(tokens[index].type), tokens[index].line, tokens[index].col);
      index++;
   }

   l_parse_program(tokens, index, &index);

   if(index == -1){
      fprintf(stderr, "Problem parsing file\n");
   }else{
      l_run_program(tokens, index);
   }

   for(int i = 0; i < index; ++i){
      l_token_free(&tokens[i]);
   }
   return 0;
}

char *l_token_str(enum token_type type){
   switch(type){
      case VAL_NUM: return "NUMBER";
      case VAL_STR: return "STRING";
      case ID: return "NAME";
      case FUNC: return "FUNC";
      case PLUS: return "PLUS";
      case PRINT: return "PRINT";
      case MINUS: return "MINUS";
      case DIV: return "DIV";
      case MULT: return "MULT";
      case NONE: return "NONE";
      default: return "NULL";
   };
}

void l_run_program(L_TOKEN *tokens, int length){
   L_TOKEN *stack[MAX_TOKENS];
   int stack_head = 0;
   L_TOKEN *first;
   L_TOKEN *second;

   for(int i = 0; i < length; ++i){
      L_TOKEN *curr = &tokens[i];
      switch(curr->type){
         case VAL_STR:
         case VAL_NUM:
            stack[stack_head++] = curr;
            break;
         case PLUS:
            first = stack[stack_head - 2];
            second  = stack[stack_head - 1];

            tokens[stack_head - 2] = (L_TOKEN){
               .type = VAL_NUM,
               .number = first->number + second->number,
            };
            stack_head--;
            break;
         case MINUS:
            first = stack[stack_head - 2];
            second  = stack[stack_head - 1];

            tokens[stack_head - 2] = (L_TOKEN){
               .type = VAL_NUM,
               .number = first->number - second->number,
            };
            stack_head--;
            break;
         case DIV:
            first = stack[stack_head - 2];
            second  = stack[stack_head - 1];

            tokens[stack_head - 2] = (L_TOKEN){
               .type = VAL_NUM,
               .number = first->number / second->number,
            };
            stack_head--;
            break;
         case MULT:
            first = stack[stack_head - 2];
            second  = stack[stack_head - 1];

            tokens[stack_head - 2] = (L_TOKEN){
               .type = VAL_NUM,
               .number = first->number * second->number,
            };
            stack_head--;
            break;
         case PRINT:
            if(stack[stack_head - 1]->type == VAL_NUM){
               printf("%f\n", stack[stack_head - 1]->number);
            }else if(stack[stack_head - 1]->type == VAL_STR){
               printf("%s\n", stack[stack_head - 1]->str);
            }else{
               printf("Cannot print variable of type %s\n", l_token_str(stack[stack_head - 1]->type));
               return;
            }
            break;
         default:
            fprintf(stderr, "Implemented token\n");
            break;
      }
   }
}

void l_parse_program(L_TOKEN *tokens, int length, int *exe_len){
   int stack_size = 0;  
   map symbols = map_create(NULL);
   for(int i = 0; i < length; ++i){
      L_TOKEN *curr = &tokens[i];
      switch(curr->type){
         case VAL_NUM:
            stack_size++;
         break;
         case PLUS:
         case MINUS:
         case MULT:
         case DIV:
            if(stack_size < 2){
               fprintf(stderr, "Problem parsing token at [%d:%d]\n", curr->line, curr->col);
               *exe_len = -1;
               break;
            }
            stack_size--;
         break;
         case PRINT:
            if(stack_size < 1){
               fprintf(stderr, "Problem parsing token at [%d:%d]\n", curr->line, curr->col);
               *exe_len = -1;
               break;
            }
            stack_size--;
         break;
         case ID:
            if(!map_contains(symbols, curr->str)) printf("Register symbol[%s]\n", curr->str);
            map_put(symbols, curr->str, curr);
         default:
            break;
      }
   }
   map_destroy(symbols);
}

int is_number(char *str){
   int dec = 0;

   for(int i = 0; i < (int)strlen(str); ++i){
      if(str[i] == '.'){
         if(dec) return 0;

         dec = 1;
      }else if(!isdigit(str[i])){
         return 0;
      }
   }

   return 1;
}
enum token_type get_token_type(char *str){
   enum token_type type = ID;
   int length = (int)strlen(str);
   if(length == 0){
      return NONE;
   }
   
   if(strcmp(str, "+") == 0){
      type = PLUS;
   }else if(strcmp(str, "-") == 0){
      type = MINUS;
   }else if(strcmp(str, "*") == 0){
      type = MULT;
   }else if(strcmp(str, "/") == 0){
      type = DIV;
   }else if(strcmp(str, "dump") == 0){
      type = PRINT;
   }else if(is_number(str)){
      type = VAL_NUM;
   }
   
   return type;
}

struct lexer{
   char *data;
   int length;
   int index;
   int empty;
   int col;
   int line;
} lexer;

void l_lexer_from_file(char *file){
   long size = 0;
   FILE *fp = fopen(file, "r");
   if(fp == NULL){
      perror("Failed to open tokens file");
      exit(-1);
   }

   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   char *data = assert_alloc(size + 1);
   
   fread(data, size, 1, fp);

   l_set_lexer(data);
}

void l_set_lexer(char *data){
   lexer.data = data;
   lexer.index = 0;
   lexer.length = (int)strlen(data);
   lexer.empty = 0;
   lexer.line = 1;
   lexer.col = 1;
}

int l_lexer_is_empty(){
   return lexer.empty;
}

L_TOKEN l_read_token(){
   char buffer[120] = {0};
   int token_size = 0;

   if(lexer.data[lexer.index] == '\0'){
      return (L_TOKEN){0};
   }

   for(; lexer.index < lexer.length && (lexer.data[lexer.index] == ' ' || lexer.data[lexer.index] == '\n'); ++lexer.index){
      if(lexer.data[lexer.index] == '\n'){
         lexer.line++;
         lexer.col = 0;
      }
      lexer.col++;
   }

   L_TOKEN token = {
      .line = lexer.line,
      .col = lexer.col,
   };

   for(; lexer.index < lexer.length; ++lexer.index){
      if(lexer.data[lexer.index] == ' ' || lexer.data[lexer.index] == '\n'){
         break;
      }
      buffer[token_size++] = lexer.data[lexer.index];
      lexer.col++;
   }


   token.type = get_token_type(buffer);

   if(token.type == VAL_NUM){
      token.number = strtod(buffer, NULL);
   }else{
      token.str = strdup(buffer);
   }

   if(lexer.data[lexer.index] == '\0'){
      lexer.empty = 1;
   }

   return token;
}

void l_delete_token(L_TOKEN *token){
   if(token->type != VAL_NUM){
      free(token->str);
   }
}
