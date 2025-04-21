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

void run_program(P_TOKEN *tokens, int length){
   STACK_VAL stack[MAX_TOKENS];
   int stack_head = 0;
   double first;
   double second;
   double number;
   int inumber;
   int id = 0;
   char *variable_name;
   map env = map_create(NULL);
/*
   printf("function tokens\n");
   for(int i = 0; i < length; ++i){
      printf("%s\n", token_str(tokens[i].type));
   }
   printf("function tokens\n\n");
*/
   for(int op_index = 0; op_index < length; ++op_index){
      P_TOKEN *curr = &tokens[op_index];
      switch(curr->type){
         case STRING:{
            fprintf(stderr, "Strings are not implemented\n");
            return;
         }
         case NUMBER:{
            stack[stack_head].val.number = curr->number;
            stack[stack_head].type = curr->type;
            DEBUGF(2, "[OP] pushed: %f", curr->number);
            stack_head++;
         }break;
         case PLUS:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f + %f", first, second);

            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first + second}
            };
            stack_head--;
         }break;
         case MINUS:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f - %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first - second}
            };
            stack_head--;
         }break;
         case DIV:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f / %f", first, second);
            stack[stack_head - 2] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){.number = first / second}
            };
            stack_head--;
         }break;
         case MULT:{
            first = stack[stack_head - 2].val.number;
            second = stack[stack_head - 1].val.number;

            DEBUGF(2, "[OP] %f * %f", first, second);
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
            number = p2d(map_get(env, variable_name));

            stack[stack_head++] = (STACK_VAL){
               .type = NUMBER,
               .val = (BYTES_8){
                  .number = number
               }
            };

            DEBUGF(2, "[OP] %s is %f", variable_name, number);
         }break;
         case DEL: {
            variable_name = get_unique_id(id, curr->name);
            map_delete_key(env, variable_name);
         } break;
         case SET_NUM:{
            variable_name = get_unique_id(id, tokens[op_index].name);
            number = stack[stack_head - 1].val.number;

            map_put(env, variable_name, d2p(number));
            DEBUGF(2, "[OP] set %s = %f", variable_name, number);
            stack_head--;
         }break;
         case FUNCTION: {
            op_index = curr->function->end;
         } break;
         case CALL: {
            const func_data *func_info = curr->function;

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

            op_index = func_info->start;
         } break;
         case RETURN: {
            inumber = stack[stack_head - 2].val.inumber;
            stack[stack_head - 2] = stack[stack_head - 1];
            stack_head--;
            // printf("returning to %d with value of %s:%f\n", inumber, token_str(stack[stack_head - 1].type), stack[stack_head - 1].val.number);
            op_index = inumber;
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
   if(token->type == ID || token->type == VAR_NUM || token->type == VAR_STR || token->type == SET_STR || token->type == SET_NUM || token->type == DEL){
      DEBUGF(3, "[MEM] free(%s)", token->str);
      free(token->str);
   }else if(token->type == FUNCTION){
      for(int i = 0; i < token->function->num_args; ++i){
         free(token->function->args[i]);
      }
      free(token->function->prefix);
      free(token->function->name);
      free(token->function->args);
      free(token->function);
   }
}


