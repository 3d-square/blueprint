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

void token_free(L_TOKEN *token){
   if(token->type != NUMBER){
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
   L_TOKEN tokens[MAX_TOKENS];
   P_TOKEN program[MAX_TOKENS];
   int index = 0;
   int program_size = 0;
   lexer_from_file("lang.l");

   while(!lexer_is_empty()){
      tokens[index] = read_token();
      if(tokens[index].type == NONE) break;
      index++;
   }

   if(parse_program(tokens, index, program, &program_size)){
      fprintf(stderr, "Parsing failed\n");
   }else{
      run_program(program, program_size);
   }

   return 0;
}
static char _unique_id[256];

char *get_unique_id(int id, char *id_name){
   sprintf(_unique_id, "%d_%s", id, id_name);

   return _unique_id;
}

void helper_run_program(P_TOKEN *tokens, int length, int id){
   STACK_VAL stack[MAX_TOKENS];
   int stack_head = 0;
   double first;
   double second;
   double number;
   char *variable_name;
   map env = map_create(NULL);

   for(int i = 0; i < length; ++i){
      P_TOKEN *curr = &tokens[i];
      DEBUGF("%s", token_str(curr->type));
      switch(curr->type){
         case STRING:
            fprintf(stderr, "Strings are not implemented\n");
            return;
         case NUMBER:
            stack[stack_head].val.number = curr->number;
            stack[stack_head].type = curr->type;
            DEBUGF("pushed: %f", curr->number);
            stack_head++;
            break;
         case PLUS:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF("%f + %f", first, second);

            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first + second}
            };
            stack_head--;
            break;
         case MINUS:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first - second}
            };
            stack_head--;
            break;
         case DIV:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            stack[stack_head - 2] = (STACK_VAL){
               .val = (BYTES_8){.number = first / second}
            };
            stack_head--;
            break;
         case MULT:
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

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

            DEBUGF("%s is %f", variable_name, number);
         break;
         case SET_NUM:
            variable_name = get_unique_id(id, tokens[i].name);
            number = stack[stack_head - 1].val.number;

            map_put(env, variable_name, d2p(number));
            DEBUGF("%s = %f", variable_name, number);
            stack_head--;
         break;
         default:
            fprintf(stderr, "Implement token %s\n", token_str(curr->type));
            exit(1);
            break;
      }
      DEBUGF("stack_size = %d", stack_head);
   }
}

void run_program(P_TOKEN *tokens, int length){
   helper_run_program(tokens, length, 0);
}

int parse_program(L_TOKEN *tokens, int length, P_TOKEN *program, int *exe_len){
   int stack_size = 0;  
   int error_status = 0;
   int stack_head = 0;
   P_TOKEN stack[MAX_TOKENS];
   *exe_len = 0;
   map symbols = map_create(NULL);
   for(int i = 0; i < length && !error_status; ++i){
      L_TOKEN *curr = &tokens[i];
      switch(curr->type){
         case NUMBER:
            stack_size++;
            program[*exe_len] = conv_token(curr);
            *exe_len = *exe_len + 1;
         break;
         case PLUS:
         case MINUS:
         case MULT:
         case DIV:
            if(stack_size < 2){
               fprintf(stderr, "Problem parsing token at [%d:%d]\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
            stack_size--;
            program[*exe_len] = conv_token(curr);
            *exe_len = *exe_len + 1;
         break;
         case PRINT:
            if(stack_size < 1){
               fprintf(stderr, "Problem parsing token at [%d:%d]\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
            program[*exe_len] = conv_token(curr);
            *exe_len = *exe_len + 1;
            stack_size--;
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
               *exe_len = *exe_len + 1;
            }

            stack[stack_head] = conv_token(name);
            stack[stack_head].type = SET_NUM;
   
            stack_head++;
         break;
         case EXPR_END:
            if(stack_head == 0){
               fprintf(stderr, "';' Expects an expression before it\n");
               error_status = 1;
               break;
            }

            program[*exe_len] = stack[stack_head - 1];

            *exe_len = *exe_len + 1;

            stack_head--;
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
               *exe_len = *exe_len + 1;
               stack_size++;
            }else{
               fprintf(stderr, "Have not implemented id of type %s\n", token_str(id->type));
               error_status = 1;
            }
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

