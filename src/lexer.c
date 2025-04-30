#include <cutils/map.h>
#include <cutils/list.h>
#include <cutils/stringbuilder.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <stdarg.h>
#include <language.h>

void *assert_alloc(size_t);

char *scatf(char *dest, const char *fmt, ...){
   va_list args;
   char buffer[1024];

   va_start(args, fmt);
   vsprintf(buffer, fmt, args);
   va_end(args);

   strcat(dest, buffer);
   return dest;
}

char *function_as_str(const func_data *function){
   static char fbuffer[1024];
   fbuffer[0] = '\0';
   scatf(fbuffer, "%s[%d]s: %d, e: %d(\n", function->name, function->num_args, function->start, function->end);
   
   for(int i = 0; i < function->num_args; ++i){
      scatf(fbuffer, "  %s\n", function->args[i]);
   }

   scatf(fbuffer, ");");

   return fbuffer;
}

char *token_str(enum token_type type){
   switch(type){
      case NUMBER: return "NUMBER";
      case STRING: return "STRING";
      case ID: return "NAME";
      case IF: return "IF";
      case IF_COND   : return "IF_COND"; 
      case ELSE_COND : return "ELSE_COND"; 
      case WHILE_COND: return "WHILE_COND"; 
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
      case END: return "END";
      case NONE: return "NONE";
      case CALL: return "CALL";
      case DEL: return "DEL";
      case COMMA: return "COMMA";
      case COND_END: return "COND_END";
      case RETURN: return "RETURN";
      case PAREN_OPEN: return "PAREN_OPEN";
      case PAREN_CLOSE: return "PAREN_CLOSE";
      case FUNCTION: return "FUNCTION";
      default: return "NULL";
   };
}

char *function_var_name(const func_data *function, const char *var){
   (void) function;
   (void) var;
   return NULL;
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
   }else if(strcmp(str, "(") == 0){
      type = PAREN_OPEN;
   }else if(strcmp(str, ")") == 0){
      type = PAREN_CLOSE;
   }else if(strcmp(str, "if") == 0){
      type = IF;
   }else if(strcmp(str, "dump") == 0){
      type = PRINT;
   }else if(strcmp(str, "del") == 0){
      type = DEL;
   }else if(strcmp(str, "else") == 0){
      type = ELSE;
   }else if(strcmp(str, "set") == 0){
      type = SET;
   }else if(strcmp(str, ";") == 0){
      type = EXPR_END;
   }else if(strcmp(str, "end") == 0){
      type = END;
   }else if(strcmp(str, ",") == 0){
      type = COMMA;
   }else if(strcmp(str, "func") == 0){
      type = FUNCTION;
   }else if(strcmp(str, "return") == 0){
      type = RETURN;
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

void lexer_from_file(const char *file){
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
   fclose(fp);
}

void lexer_destroy(){
   free(lexer.data);
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

   if( strncmp(curr, "+", 1) == 0
    || strncmp(curr, "-", 1) == 0
    || strncmp(curr, "*", 1) == 0
    || strncmp(curr, "/", 1) == 0
    || strncmp(curr, ";", 1) == 0 
    || strncmp(curr, "(", 1) == 0
    || strncmp(curr, ",", 1) == 0
    || strncmp(curr, ")", 1) == 0){
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
        // DEBUGF( "[MEM] strdup(%s)", token.str);
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
   }else if(token_size > 0){
      token.str = strdup(buffer);
      // DEBUGF( "[MEM] strdup(%s)", token.str);
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
