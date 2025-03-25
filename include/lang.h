#pragma once

#include <stdio.h>

enum token_type{
   VAL_NUM,
   VAL_STR,
   ID,
   FUNC,
   PLUS,
   MINUS,
   DIV,
   MULT,
   PRINT,
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

L_TOKEN l_read_token();

void l_lexer_from_file(char *file);

void l_set_lexer(char *data);

void l_delete_token(L_TOKEN *token);

int l_lexer_is_empty();

void l_parse_program(L_TOKEN *tokens, int length, int *exe_len);

void l_run_program(L_TOKEN *tokens, int length);

char *l_token_str(enum token_type);
