#include "lang.h"
#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <lexer.h>

char *get_function_prefix(P_TOKEN stack[], int size, int nested, char *curr);
int token_prec(enum token_type type);
P_TOKEN conv_token(L_TOKEN *token);

void *assert_alloc(size_t size);

int parse_program(L_TOKEN *tokens, int length, P_TOKEN *program, int *exe_len){
   int stack_size = 0;  
   int error_status = 0;
   int stack_head = 0;
   P_TOKEN stack[MAX_TOKENS];
   P_TOKEN expression_stack[MAX_TOKENS];
   int in_function = 0;
   int expr_size = 0;
   int last_was_op = 1;
   *exe_len = 0;
   map symbols = map_create(NULL);
   for(int op_index = 0; op_index < length && !error_status; ++op_index){
      L_TOKEN *curr = &tokens[ op_index];
      switch(curr->type){
         case NUMBER:{
            stack_size++;
            program[*exe_len] = conv_token(curr);
            ++(*exe_len);
            last_was_op = 0;
         }break;
         case PLUS:
         case MINUS:
         case MULT:
         case DIV:{
            if(op_index < length - 1 && tokens[op_index + 1].type == EXPR_END){
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
         }break;
         case PRINT:{
            if(stack_size < 1){
               fprintf(stderr, "Problem parsing token %s at [%d:%d]\n", token_str(curr->type), curr->line, curr->col);
               error_status = 1;
               break;
            }
            program[*exe_len] = conv_token(curr);
            ++(*exe_len);
            stack_size--;
         }break;
         case PAREN_OPEN:{
            expression_stack[expr_size++] = conv_token(curr);
         }break;
         case PAREN_CLOSE:{
            while(expr_size > 0 && expression_stack[expr_size - 1].type != PAREN_OPEN){
               program[*exe_len] = expression_stack[--expr_size];
               ++(*exe_len);
            }

            if(expr_size < 1){
               fprintf(stderr, "'(' at [%d:%d] does not have a corresponding ')'\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
            free(curr->str);
            expr_size--;
         }break;
         case SET:{
            if(op_index + 2 >= length){
               fprintf(stderr, "%d:%d set expects variable name and ';'\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
            L_TOKEN *name = &tokens[++op_index];
            if(name->type != ID){
               fprintf(stderr, "%d:%d set expects ID but got type %s\n", name->line, name->col, token_str(name->type));
               error_status = 1;
               break;
            }
            int next_idx = op_index + 1;
            /* Look for EXPR_END */
            while(next_idx < length && tokens[next_idx].type != EXPR_END){
               if(tokens[next_idx].type == ID && strcmp(name->str, tokens[next_idx].str) == 0){
                  L_TOKEN *self_rec = (L_TOKEN *)map_get(symbols, name->str);
                  if(self_rec == NULL){
                     fprintf(stderr, "Initial definition of '%s' at [%d:%d] is recursive\n", name->str, name->line, name->col);
                     error_status = 1;
                     goto ERR;
                  }
               }
               ++next_idx;
            }

            if(next_idx == length){
               fprintf(stderr, "%d:%d set '%s' ran into EOF while looking for ';'\n", curr->line, curr->col, name->str);
               error_status = 1;
               break;
            }

            name->type = SET_NUM;
            map_put(symbols, name->str, (void *)SET_NUM);

            if(next_idx == op_index + 1){
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
         }break;
         case EXPR_END:{
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

            if(stack[stack_head - 1].type == SET_NUM){
               stack_head--;
            }
            last_was_op = 1;
         }break;
         case ID:{
            char buffer[256];
            enum token_type id = NULL_TOKEN;
            sprintf(buffer, "%s_%s", get_function_prefix(stack, stack_head, in_function, ""), curr->str);

            id = (enum token_type)map_get(symbols, buffer);
            if(id == NULL_TOKEN){
               id = (enum token_type)map_get(symbols, curr->str);
            }else{ // variable is a function variable
               free(curr->str);
               curr->str = strdup(buffer);
            }
            
            if(id == NULL_TOKEN){
               fprintf(stderr, "ID: symbol[%s] is neither function nor variable\n", curr->str);
               error_status = 1;
               break;
            }

            if(id == SET_NUM){
               program[*exe_len] = conv_token(curr);
               program[*exe_len].type = VAR_NUM;
               ++(*exe_len);
               stack_size++;
            }else{
               fprintf(stderr, "ID: Have not implemented id of type %s\n", token_str(id));
               error_status = 1;
            }

            last_was_op = 0;
         } break;
         case FUNCTION:{
            if(op_index + 6 >= length){
               fprintf(stderr, "[%d:%d]: Incomplete function definiiton: function definition is required to use the following format\n"
                               "  func function_name(var1, ..., varN)\n"
                               "    ...\n"
                               "  end\n", curr->line, curr->col);
               error_status = 1;
               break;
            }

            L_TOKEN *func = &tokens[op_index + 1];
            if(func->type != ID || tokens[op_index + 2].type != PAREN_OPEN){
               fprintf(stderr, "[%d:%d]: Function definition is required to use the following format\n"
                               "  func function_name(var1, ..., varN)\n"
                               "    ...\n"
                               "  end\n", curr->line, curr->col);
               error_status = 1;
               break;
            }

            int j;
            int num_args = 0;
            char *arg_names[25];
            char arg_name_buffer[256];
            char *function_name = func->str;
            char *prefix = strdup(get_function_prefix(stack, stack_head, in_function, function_name));

            for(j = op_index + 3; j < length - 1; j += 2){
               L_TOKEN *var = &tokens[j];
               L_TOKEN *next = &tokens[j + 1];

               if(var->type != ID){
                  fprintf(stderr, "[%d:%d]: Function definition expects ID not %s\n", var->line, var->col, token_str(var->type));
                  error_status = 1;
                  goto ERR;
               }
               var->type = SET_NUM;
               sprintf(arg_name_buffer, "%s_%s", prefix, var->str);
               arg_names[num_args++] = arg_name_buffer;
               map_put(symbols, arg_name_buffer, (void *)SET_NUM);
               if(next->type == COMMA){
                  // There will be more variables
                  free(next->str);
               }else if(next->type == PAREN_CLOSE){
                  // There are no more variables
                  free(next->str);
                  break;
               }else{
                  fprintf(stderr, "[%d:%d]: Function definition expects ID not %s\n", next->line, next->col, token_str(var->type));
                  error_status = 1;
                  goto ERR;
               }
            }

            if(j >= length){
               fprintf(stderr, "Ran into End Of File while parsing function\n");
               error_status = 1;
               break;
            }

            in_function++;

            func_data *function_data = assert_alloc(sizeof(func_data));
            function_data->name = function_name;
            function_data->num_args = num_args;
            function_data->prefix = prefix;
            function_data->args = assert_alloc(num_args * sizeof(char *));

            for(int k = 0; k < num_args; ++k){
               function_data->args[k] = arg_names[k];
            }

            func->function_data = function_data;

            map_put(symbols, function_name, func);

            func->type = FUNCTION;
            P_TOKEN func_header = conv_token(func);
            stack[stack_head++] = func_header;
            program[*exe_len] = func_header;
            ++(*exe_len);

            op_index += j + 1;
         } break;
         case END: {
            if(stack_head <= 0){
               fprintf(stderr, "[%d:%d]: end expects function definition\n", curr->line, curr->col);
               error_status = 1;
               break;
            }
            P_TOKEN func = stack[--stack_head];
            if(func.type != FUNCTION){
               fprintf(stderr, "[%d:%d]: Function ended in the middle of another statement. Got type %s\n", curr->line, curr->col, token_str(func.type));
               error_status = 1;
               break;
            }
            in_function--;
            for(int j = 0; j < func.function->num_args; ++j){
               map_delete_key(symbols, func.function->args[j]);
            }
         } break;
         default:
            fprintf(stderr, "[%d:%d]: Cannot parse %s\n", curr->line, curr->col, token_str(curr->type));
            exit(1);
            break;
      }

      ERR:
   }

   if(stack_head > 0 && error_status == 0){
      for(int i =  stack_head - 1; i >= 0; --i){
         fprintf(stderr, "%s expects ';'\n", token_str(stack[i].type));
      }
      error_status = 1;
   }
   // printf("map_size: %d\n", symbols->size);
   // for(map_iter iter = map_create_iter(symbols); map_has_next(&iter); map_iter_next(&iter)){
   //    key_val *curr = map_iter_data(&iter);
   //    if(curr == NULL) {
   //       fprintf(stderr, "Iterator Exception\n");
   //       exit(1);
   //    }
   //    printf("env['%s'] = %p\n", curr->key, curr->val);
   // }

   map_destroy(symbols);
   exit(1);
   return error_status;
}

void *assert_alloc(size_t size){
   void *var = calloc(1, size);
   if(var == NULL){
      perror("Malloc Fail\n"); exit(-1);
   }
   return var;
}

char *get_function_prefix(P_TOKEN stack[], int size, int nested, char *curr){
   if(nested == 0){
      return curr;
   }

   static char prefix[256];
   int i;

   prefix[0] = '\0';

   for(i = size - 1; i >= 0; --i){
      if(stack[i].type == FUNCTION){
         nested--;
      }

      if(nested == 0) break;
   }

   if(nested != 0){
      fprintf(stderr, "Problem computing prefix\n");
      exit(1);
   }

   for(; i < size; ++i){
      if(stack[i].type == FUNCTION){
         strcat(prefix, stack[i].function->name);
      }
   }

   strcat(prefix, curr);

   return prefix;
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
      case FUNCTION:
         new_token.function = token->function_data;
      break;
      default:
         fprintf(stderr, "%s token conversion is not implemented yet\n", token_str(token->type));
         exit(1);
      break;
      
   }

   return new_token;
}


