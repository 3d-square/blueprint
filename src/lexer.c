#include "lang.h"
#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <lexer.h>

void *assert_alloc(size_t);

char *token_str(enum token_type type){
   switch(type){
      case NUMBER: return "NUMBER";
      case STRING: return "STRING";
      case ID: return "NAME";
      case SET: return "SET";
      case SET_NUM: return "SET_NUM";
      case VAR_NUM: return "VAR_NUM";
      case EXPR_END: return "EXPR_END";
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
   }else if(strcmp(str, "set") == 0){
      type = SET;
   }else if(strcmp(str, ";") == 0){
      type = EXPR_END;
   }else if(is_number(str)){
      type = NUMBER;
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
   int found_next;
} lexer;

void lexer_from_file(char *file){
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

   set_lexer(data);
}

void set_lexer(char *data){
   lexer.data = data;
   lexer.index = 0;
   lexer.length = (int)strlen(data);
   lexer.empty = 0;
   lexer.line = 1;
   lexer.col = 1;
}

int lexer_is_empty(){
   return lexer.empty;
}

int end_of_token(){
   int end = 0;
   lexer.found_next = 0;
   if(lexer_is_empty()){
      return 0;
   }

   char *curr = lexer.data + lexer.index + 1;

   if(strncmp(curr, "+=", 2) == 0){
      end = 1;
      lexer.found_next = 2;
   }else if(strncmp(curr, "+", 1) == 0){
      end = 1;
      lexer.found_next = 1;
   }else if(strncmp(curr, "-", 1) == 0){
      end = 1;
      lexer.found_next = 1;
   }else if(strncmp(curr, "*", 1) == 0){
      end = 1;
      lexer.found_next = 1;
   }else if(strncmp(curr, "/", 1) == 0){
      end = 1;
      lexer.found_next = 1;
   }else if(strncmp(curr, ";", 1) == 0){
      end = 1;
      lexer.found_next = 1;
   }

   return end;
}

L_TOKEN read_token(){
   char buffer[120] = {0};
   int token_size = 0;

   if(lexer.data[lexer.index] == '\0'){
      return (L_TOKEN){0};
   }

  if(lexer.found_next != 0){
     L_TOKEN token = {
        .line = lexer.line,
        .col = lexer.col,
     };
     lexer.col += lexer.found_next;
     for(int i = 0; i < lexer.found_next; ++i, ++lexer.index){
        buffer[i] = lexer.data[lexer.index];
     }

     lexer.found_next = 0;
     token.type = get_token_type(buffer);

     if(token.type == NUMBER){
        token.number = strtod(buffer, NULL);
     }else{
        token.str = strdup(buffer);
     }

     if(lexer.data[lexer.index] == '\0'){
        lexer.empty = 1;
     }
     return token;
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

      if(end_of_token()){
         ++lexer.index;
         break;
      }
   }

   token.type = get_token_type(buffer);

   if(token.type == NUMBER){
      token.number = strtod(buffer, NULL);
   }else{
      token.str = strdup(buffer);
   }

   if(lexer.data[lexer.index] == '\0'){
      lexer.empty = 1;
   }

   return token;
}

void delete_token(L_TOKEN *token){
   if(token->type != NUMBER){
      free(token->str);
   }
}
