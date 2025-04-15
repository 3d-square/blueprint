#pragma once

#include <stdio.h>
#include <lexer.h>
#include <cutils/list.h>

#define MAX_TOKENS 1024

typedef struct _p_token{
   enum token_type type;
   union {
      double number;
      func_data *function;
      char *str;
      char *name;
   };
} P_TOKEN;

typedef union _bytes_8{
   double number;
   void *data;
   char *str;
   P_TOKEN *instr;
} BYTES_8;

typedef struct _stack_val{
   enum token_type type;
   BYTES_8 val;
} STACK_VAL;

int read_parse_file(char *file_name, P_TOKEN *program);

int parse_program(L_TOKEN *tokens, int length, P_TOKEN *program, int *exe_len);

void run_program(P_TOKEN *tokens, int length);

P_TOKEN conv_token(L_TOKEN *token);

// void save_program(P_TOKEN *tokens, int length, char *file_name);
// void load_program(P_TOKEN *tokens, int length, char *file_name);
