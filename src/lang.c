#include "lang.h"
#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <lexer.h>

#define MAX_TOKENS 1024

void *assert_alloc(size_t size){
   void *var = calloc(1, size);
   if(var == NULL){
      perror("Malloc Fail\n"); exit(-1);
   }
   return var;
}

void token_free(P_TOKEN *token){
   if(token->type == ID || token->type == VAR_NUM || token->type == VAR_STR || token->type == SET_STR || token->type == SET_NUM){
      DEBUGF(3, "[MEM] free(%s)", token->str);
      free(token->str);
   }
}

P_TOKEN conv_token(L_TOKEN *token){
   P_TOKEN new_token = {
      .type = token->type,
      .str = NULL,
   };

   switch(token->type){
      case PLUS:
      case MINUS:
      case DIV:
      case MULT:
      case SET:
      case EXPR_END:
      case PRINT:
      case PAREN_OPEN:
      case PAREN_CLOSE:
         DEBUGF(3, "[MEM] free(%s)", token->str);
         free(token->str);
      break;
      case NUMBER:
         new_token.number = token->number;
      break;
      case SET_NUM:
      case ID:
         new_token.name = token->str;
      break;
      default:
         fprintf(stderr, "%s token conversion is not implemented yet\n", token_str(token->type));
         exit(1);
      break;
      
   }

   return new_token;
}

int main(){
   P_TOKEN program[MAX_TOKENS];
   int size;

   if((size = read_parse_file("lang.l", program)) != -1){
      run_program(program, size);
   }

   return 0;
}
static char _unique_id[256];

char *get_unique_id(int id, char *id_name){
   sprintf(_unique_id, "%d_%s", id, id_name);

   return _unique_id;
}

void run_program(P_TOKEN *tokens, int length){
   STACK_VAL stack[MAX_TOKENS];
   int stack_head = 0;
   double first;
   double second;
   double number;
   int id = 0;
   char *variable_name;
   map env = map_create(NULL);

   for(int i = 0; i < length; ++i){
      P_TOKEN *curr = &tokens[i];
      switch(curr->type){
         case STRING:
            fprintf(stderr, "Strings are not implemented\n");
            return;
         case NUMBER:
            stack[stack_head].val.number = curr->number;
            stack[stack_head].type = curr->type;
            DEBUGF(2, "[OP] pushed: %f", curr->number);
            stack_head++;
            break;
         case PLUS:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f + %f", first, second);

            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first + second}
            };
            stack_head--;
            break;
         case MINUS:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f - %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first - second}
            };
            stack_head--;
            break;
         case DIV:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f / %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first / second}
            };
            stack_head--;
            break;
         case MULT:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f * %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first * second}
            };
            stack_head--;
            break;
         case PRINT:
            if(stack[stack_head - 1].type == NUMBER){
               printf("%f\n", stack[stack_head - 1].val.number);
            }else{
               fprintf(stderr, "Value type %s is not implmented to print\n", token_str(stack[stack_head - 1].type));
            }
            stack_head--;
            break;
         case VAR_NUM:
            variable_name = get_unique_id(id, tokens[i].name);
            number = p2d(map_get(env, variable_name));

            stack[stack_head++] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){
                  .number = number
               }
            };

            DEBUGF(2, "[OP] %s is %f", variable_name, number);
         break;
         case SET_NUM:
            variable_name = get_unique_id(id, tokens[i].name);
            number = stack[stack_head - 1].val.number;

            map_put(env, variable_name, d2p(number));
            DEBUGF(2, "[OP] set %s = %f", variable_name, number);
            stack_head--;
         break;
         default:
            fprintf(stderr, "Implement token %s\n", token_str(curr->type));
            exit(1);
            break;
      }
      DEBUGF(2, "[PROGRAM] stack_size = %d", stack_head);
   }
   map_destroy(env);

   for(int i = 0; i < length; ++i){
      token_free(&tokens[i]);
   }
}

int token_prec(enum token_type type){
   switch(type){
      case PLUS:
      case MINUS:
         return 1;

      case MULT:
      case DIV:
         return 2;
      case PAREN_OPEN:
         return -1;

      default:
         fprintf(stderr, "Type %s is not a supported expression operator type\n", token_str(type));
         exit(-1);
   }
}

int parse_program(L_TOKEN *tokens, int length, P_TOKEN *program, int *exe_len){
   int stack_size = 0;  
   int error_status = 0;
   int stack_head = 0;
   P_TOKEN stack[MAX_TOKENS];
   P_TOKEN expression_stack[MAX_TOKENS];
   int expr_size = 0;
   int last_was_op = 1;
   *exe_len = 0;
   map symbols = map_create(NULL);
   for(int i = 0; i < length && !error_status; ++i){
      L_TOKEN *curr = &tokens[i];
      switch(curr->type){
         case NUMBER:
            stack_size++;
            program[*exe_len] = conv_token(curr);
            ++(*exe_len);
            last_was_op = 0;
         break;
         case PLUS:
         case MINUS:
         case MULT:
         case DIV:
            if(i < length - 1 && program[i + 1].type == EXPR_END){
               fprintf(stderr, "Operand %s expects NUMBER or ID but got EXPR_END ';'\n", token_str(curr->type));
               error_status = 1;
               break;
            }
            if(last_was_op){
               fprintf(stderr, "Two adjacent operands found at [%d:%d]\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
           stack_size--;

            // Pop all expressions from stack and add to program
            while(expr_size && token_prec(curr->type) <= token_prec(expression_stack[expr_size - 1].type)){
               program[*exe_len] = expression_stack[expr_size - 1];
               ++(*exe_len);
               expr_size--;
            }

            expression_stack[expr_size++] = conv_token(curr);
            last_was_op = 1;
         break;
         case PRINT:
            if(stack_size < 1){
               fprintf(stderr, "Problem parsing token %s at [%d:%d]\n", token_str(curr->type), curr->line, curr->col);
               error_status = 1;
               break;
            }
            program[*exe_len] = conv_token(curr);
            ++(*exe_len);
            stack_size--;
         break;
         case PAREN_OPEN:
            expression_stack[expr_size++] = conv_token(curr);
         break;
         case PAREN_CLOSE:
            while(expr_size > 0 && expression_stack[expr_size - 1].type != PAREN_OPEN){
               program[*exe_len] = expression_stack[--expr_size];
               ++(*exe_len);
            }

            if(expr_size < 1){
               fprintf(stderr, "'(' at [%d:%d] does not have a corresponding ')'\n", curr->line, curr->col);
               error_status = 1;
               break;
            }

            expr_size--;
         break;
         case SET:
            if(i + 2 >= length){
               fprintf(stderr, "%d:%d set expects variable name and ';'\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
            L_TOKEN *name = &tokens[++i];
            if(name->type != ID){
               fprintf(stderr, "%d:%d set expects ID but got type %s\n", name->line, name->col, token_str(name->type));
               error_status = 1;
               break;
            }
            int next_idx = i + 1;
            /* Look for EXPR_END */
            while(next_idx < length && tokens[next_idx].type != EXPR_END){
               ++next_idx;
            }

            if(next_idx == length){
               fprintf(stderr, "%d:%d set '%s' ran into EOF while looking for ';'\n", curr->line, curr->col, name->str);
               error_status = 1;
               break;
            }

            name->type = SET_NUM;
            map_put(symbols, name->str, name);

            if(next_idx == i + 1){
               program[*exe_len] = (P_TOKEN){
                  .type = NUMBER,
                  .number = 0,
               };
               ++(*exe_len);
            }

            stack[stack_head] = conv_token(name);
            stack[stack_head].type = SET_NUM;
   
            stack_head++;

            DEBUGF(3, "[MEM] free(%s)", curr->str);
            free(curr->str);
         break;
         case EXPR_END:
            if(stack_head == 0){
               fprintf(stderr, "';' Expects an expression before it\n");
               error_status = 1;
               break;
            }

            DEBUGF(3, "[MEM] free(%s)", curr->str);
            free(curr->str);

            while(expr_size){
               program[*exe_len] = expression_stack[expr_size - 1];
               ++(*exe_len);
               expr_size--;
            }

            program[*exe_len] = stack[stack_head - 1];

            ++(*exe_len);

            stack_head--;
            last_was_op = 1;
         break;
         case ID:
            L_TOKEN *id = (L_TOKEN *)map_get(symbols, curr->str);
            if(id == NULL){
               fprintf(stderr, "symbol[%s] is neither function nor variable\n", curr->str);
               error_status = 1;
               break;
            }

            if(id->type == SET_NUM){
               program[*exe_len] = conv_token(curr);
               program[*exe_len].type = VAR_NUM;
               ++(*exe_len);
               stack_size++;
            }else{
               fprintf(stderr, "Have not implemented id of type %s\n", token_str(id->type));
               error_status = 1;
            }

            last_was_op = 0;
         default:
            break;
      }
   }

   if(stack_head > 0 && error_status == 0){
      for(int i =  stack_head - 1; i >= 0; --i){
         fprintf(stderr, "%s expects ';'\n", token_str(stack[i].type));
      }
      error_status = 1;
   }

   map_destroy(symbols);
   return error_status;
}

int read_parse_file(char *file_name, P_TOKEN *program){
   L_TOKEN tokens[MAX_TOKENS];
   int index = 0;
   int program_size = 0;
   lexer_from_file(file_name);

   while(!lexer_is_empty()){
      tokens[index] = read_token();
      if(tokens[index].type == NONE) break;
      index++;
   }

   DEBUGF(1, "[PROGRAM] Num Tokens: %d", index);
   lexer_destroy();
   if(parse_program(tokens, index, program, &program_size)){
      fprintf(stderr, "Parsing failed\n");
      return -1;
   }

   DEBUGF(1, "[PROGRAM] Program size: %d", program_size);
   return program_size;
}


