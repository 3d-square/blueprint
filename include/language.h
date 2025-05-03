#pragma once 

#include <stdio.h>
#include <cutils/list.h>
#include <cutils/stringbuilder.h>

#define MAX_TOKENS 1024

// Lexer/Tokenizer Functions

enum token_type{
   NULL_TOKEN,
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
   FUNCTION,
   END,
   COMMA,
   RETURN,
   CALL,
   DEL,
   IF,
   IF_COND,
   ELSE,
   ELSE_COND,
   WHILE_COND,
   COND_END,
   NONE
};

typedef struct _func_data{
   int start;
   int end;
   int num_args;
   char **args;
   char *name;
   char *prefix;
} func_data;

typedef struct cond_{
   int start;
   int end;
   int next;
} cond_data;

typedef struct _token{
   enum token_type type;
   int col;
   int line;
   union {
      double number;
      char *str;
      func_data *function_data;
      cond_data *cond_info;
   };
} L_TOKEN;

typedef struct _p_token{
   enum token_type type;
   int line;
   union {
      double number;
      func_data *function;
      cond_data *conditional;
      char *str;
      char *name;
   };
} P_TOKEN;

typedef union _bytes_8{
   double number;
   int inumber;
   void *data;
   char *str;
   P_TOKEN *instr;
} BYTES_8;

typedef struct _stack_val{
   enum token_type type;
   BYTES_8 val;
} STACK_VAL;

struct debugger{
   int line;
   char op;
};

#define token_errorf(fmt, tkn, ...) fprintf(stderr, "[%d:%d]: " fmt "\n      '%s'\n", (tkn)->line, (tkn)->col, __VA_ARGS__, sb_str(&(file_lines[(tkn)->line - 1]), _sb_line_buffer, sizeof(_sb_line_buffer)))
#define token_error(fmt, tkn) fprintf(stderr, "[%d:%d]: " fmt "\n      '%s'\n", (tkn)->line, (tkn)->col, sb_str(&(file_lines[(tkn)->line - 1]), _sb_line_buffer, sizeof(_sb_line_buffer)))

extern sb *file_lines;
extern size_t num_lines;

extern char _sb_line_buffer[1024];

char *function_as_str(const func_data *function);

char *token_str(enum token_type);

L_TOKEN read_token();

void lexer_from_file(const char *file);

void set_lexer(/*const */char *data);

void lexer_destroy();

void delete_token(L_TOKEN *token);

int lexer_is_empty();

int end_of_token();

char *function_var_name(const func_data *function, const char *var);

int read_parse_file(const char *file_name, P_TOKEN *program);

int parse_program(L_TOKEN *tokens, int length, P_TOKEN *program, int *exe_len);

void run_program(P_TOKEN *tokens, int length, int debug);

P_TOKEN conv_token(L_TOKEN *token);

char *get_function_prefix(P_TOKEN stack[], int size, int nested, char *curr);

int token_prec(enum token_type type);

P_TOKEN conv_token(L_TOKEN *token);

char *id_search(char *name, P_TOKEN *stack, int stack_head, int in_func, map env, enum token_type *id);

