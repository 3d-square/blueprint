#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <language.h>

char *get_unique_id(int id, char *id_name);
void token_free(P_TOKEN *token);

#define runtime_errorf(msg, ...) { fprintf(stderr, "[RUN TIME ERROR]: " msg "\n", __VA_ARGS__); exit(1); }

void debug_opt(STACK_VAL stack[], int stack_head, int program_opt, P_TOKEN *curr_val, map env);

void run_program(P_TOKEN *tokens, int length, int debug){
   STACK_VAL stack[MAX_TOKENS];
   int stack_head = 0;
   double first;
   double second;
   double number;
   int inumber;
   int id = 0;
   char *variable_name;
   map env = map_create(NULL);

   DEBUG( "\nfunction tokens\n");
   for(int i = 0; i < length; ++i){
      DEBUGF( "%s", token_str(tokens[i].type));
   }
   DEBUG( "\nfunction tokens\n\n");

   for(int op_index = 0; op_index < length; ++op_index){
      P_TOKEN *curr = &tokens[op_index];
      if(debug) debug_opt(stack, stack_head, op_index, curr, env);
      switch(curr->type){
         case STRING:{
            fprintf(stderr, "Strings are not implemented\n");
            return;
         }
         case NUMBER:{
            stack[stack_head].val.number = curr->number;
            stack[stack_head].type = curr->type;
            DEBUGF( "[OP] pushed: %f", curr->number);
            stack_head++;
         }break;
         case PLUS:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF( "[OP] %f + %f", first, second);

            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first + second}
            };
            stack_head--;
         }break;
         case MINUS:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF( "[OP] %f - %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first - second}
            };
            stack_head--;
         }break;
         case DIV:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF( "[OP] %f / %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first / second}
            };
            stack_head--;
         }break;
         case MULT:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF( "[OP] %f * %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first * second}
            };
            stack_head--;
         }break;
         case PRINT:{
            if(stack[stack_head - 1].type == NUMBER){
               printf("%f\n", stack[stack_head - 1].val.number);
            }else{
               fprintf(stderr, "Value type %s is not implmented to print\n", token_str(stack[stack_head - 1].type));
            }
            stack_head--;
         }break;
         case VAR_NUM:{
            variable_name = get_unique_id(id, curr->name);
            if(!map_contains(env, variable_name)){
               runtime_errorf("Variable %s has been deleted prior to accessing it", variable_name);
            }
            number = p2d(map_get(env, variable_name));

            stack[stack_head++] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){
                  .number = number
               }
            };

            DEBUGF( "[OP] %s is %f", variable_name, number);
         }break;
         case DEL: {
            variable_name = get_unique_id(id, curr->name);
            map_delete_key(env, variable_name);
         } break;
         case SET_NUM:{
            variable_name = get_unique_id(id, tokens[op_index].name);
            number = stack[stack_head - 1].val.number;

            map_put(env, variable_name, d2p(number));
            DEBUGF( "[OP] set %s = %f", variable_name, number);
            stack_head--;
         }break;
         case FUNCTION: {
            op_index = curr->function->end;
         } break;
         case CALL: {
            const func_data *func_info = curr->function;
            DEBUGF("Calling function %s", func_info->name);
            for(int i = 0; i < func_info->num_args; ++i){
               // printf("variable %s is %f\n", func_info->args[i], stack[stack_head - func_info->num_args + i].val.number);
               map_put(env, get_unique_id(id, func_info->args[i]), d2p(stack[stack_head - func_info->num_args + i].val.number));
            }
            stack_head -= func_info->num_args;
            stack[stack_head++] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){
                  .inumber = op_index
               }
            };
            printf("return to %d\n", op_index);

            op_index = func_info->start;
         } break;
         case RETURN: {
            inumber = stack[stack_head - 2].val.inumber;
            printf("returning to %d\n", inumber);
            stack[stack_head - 2] = stack[stack_head - 1];
            stack_head--;
            // printf("returning to %d with value of %s:%f\n", inumber, token_str(stack[stack_head - 1].type), stack[stack_head - 1].val.number);
            op_index = inumber;
         } break;
         case IF_COND: {
            int cond = (int)stack[--stack_head].val.number;
            if(!cond){
               op_index = curr->conditional->next;
            }
            
         } break;
         case COND_END: {
            op_index = curr->conditional->end;
         } break;
         default:
            fprintf(stderr, "%d: Implement token %s[%d]\n", op_index, token_str(curr->type), curr->type);
            exit(1);
      }
   }
   map_destroy(env);

   for(int i = 0; i < length; ++i){
      token_free(&tokens[i]);
   }
}

char *get_unique_id(int id, char *id_name){
   static char _unique_id[256];
   sprintf(_unique_id, "%d_%s", id, id_name);
   return _unique_id;
}

void token_free(P_TOKEN *token){
   if(token->type == ID || token->type == VAR_NUM || token->type == VAR_STR || token->type == SET_STR || token->type == SET_NUM /* || token->type == DEL */){
      DEBUGF( "[MEM] free(%s)", token->str);
      free(token->str);
   }else if(token->type == FUNCTION){
      for(int i = 0; i < token->function->num_args; ++i){
         free(token->function->args[i]);
      }
      free(token->function->prefix);
      free(token->function->name);
      free(token->function->args);
      free(token->function);
   }else if(token->type == IF_COND){
      free(token->conditional);
   }
}

void debug_opt(STACK_VAL stack[], int stack_head, int program_opt, P_TOKEN *curr_val, map env){
   char command[80];
   static int line = 0;
   static int run = 0;
   static int breakpoints[100];
   static int num_breakpoints = 0;

   for(int i = 0; i < num_breakpoints; ++i){
      if(program_opt == breakpoints[i]){
         run = 0;
         break;
      }
   }

   int b = 0;

   if(run){
      if(line != curr_val->line){
         run = 0;
         line = curr_val->line;
      }else{
         return;
      }
   }
   int repeat = 1;

   while(repeat){
      fgets(command, sizeof(command), stdin);
      repeat = 0;

      if(strncmp(command, "line", 4) == 0){
         if(curr_val->line != -1){
            printf("line[%d]: %s\n", curr_val->line, sb_str(&file_lines[curr_val->line - 1], _sb_line_buffer, sizeof(_sb_line_buffer)));
         }
      }else if(strncmp(command, "run", 3) == 0){
         run = 1;
         line = curr_val->line;
         printf("ran line: %s\n", sb_str(&file_lines[curr_val->line - 1], _sb_line_buffer, sizeof(_sb_line_buffer)));
      }else if(strncmp(command, "stack", 5) == 0){
         printf("Stack size: %d\n", stack_head);
         for(int i = stack_head - 1; i >= 0; --i){
            if(stack[i].type == NUMBER){
               printf("%f = %d\n", stack[i].val.number, stack[i].val.inumber);
            }else{
               printf("Stack type %s is not implemented\n", token_str(stack[i].type));
               exit(1);
            }
         }
      }else if(sscanf(command, "break %d", &b) == 1){
         breakpoints[num_breakpoints++] = b;
         repeat = 1;
      }else{
         program_opt++;
         (void)env;
      }
   }
}
