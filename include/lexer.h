#pragma once 

#include <stdio.h>
#include <cutils/list.h>

enum token_type{
   NUMBER,
   SET_NUM,
   VAR_NUM,
   STRING,
   SET_STR,
   VAR_STR,
   ID,
   SET,
   FUNC,
   PLUS,
   MINUS,
   DIV,
   MULT,
   PRINT,
   PAREN_OPEN,
   PAREN_CLOSE,
   EXPR_END,
   NONE
};

typedef struct _token{
   enum token_type type;
   int col;
   int line;
   union {
      double number;
      char *str;
   };
} L_TOKEN;

struct func_data{
   char *name;
   int num_args;
   union {
      double number;
      void *data;
   } result;
};
char *token_str(enum token_type);

L_TOKEN read_token();

void lexer_from_file(char *file);

void set_lexer(char *data);

void lexer_destroy();

void delete_token(L_TOKEN *token);

int lexer_is_empty();

int end_of_token();
