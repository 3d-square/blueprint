#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <time.h>
#include <language.h>

void *assert_alloc(size_t size);

char *get_random_str(int size);

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
   map functions = map_create(NULL);
   srand(142);

   for(int op_index = 0; op_index < length && !error_status; ++op_index){
      L_TOKEN *curr = &tokens[ op_index];
      switch(curr->type){
         case NUMBER:{
            if(last_was_op == 0){
               token_error("Expected operand instead got number", curr);
               error_status = 1;
               break;
            }
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
               token_errorf("Operand %s expects NUMBER or ID but got EXPR_END ';'", curr, token_str(curr->type));
               error_status = 1;
               break;
            }
            if(last_was_op){
               token_error("Two adjacent operands found", curr);
               error_status = 1;
               break;
            }
           stack_size--;

            // Pop all expressions from stack and add to program
            while(expr_size && token_prec(curr->type) <= token_prec(expression_stack[expr_size - 1].type)){
               program[*exe_len] = expression_stack[--expr_size];
               ++(*exe_len);
            }

            expression_stack[expr_size++] = conv_token(curr);
            last_was_op = 1;
         }break;
         case PRINT:{
            if(stack_size < 1){
               token_errorf("Problem parsing token %s\n", curr, token_str(curr->type));
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
               token_error("'(' does not have a corresponding ')'", curr);
               error_status = 1;
               break;
            }
            free(curr->str);
            expr_size--;
         }break;
         case SET:{
            if(op_index + 2 >= length){
               token_error("Set expects variable name and ';'\n", curr);
               error_status = 1;
               break;
            }
            L_TOKEN *name = &tokens[++op_index];
            if(name->type != ID){
               token_errorf("Set expects ID but got type %s\n", name, token_str(name->type));
               error_status = 1;
               break;
            }
            int next_idx = op_index + 1;
            /* Look for EXPR_END */
            while(next_idx < length && tokens[next_idx].type != EXPR_END){
               if(tokens[next_idx].type == ID && strcmp(name->str, tokens[next_idx].str) == 0){
                  L_TOKEN *self_rec = (L_TOKEN *)map_get(symbols, name->str);
                  if(self_rec == NULL){
                     token_errorf("Initial definition of '%s' is recursive", name, name->str);
                     error_status = 1;
                     goto ERR;
                  }
               }
               ++next_idx;
            }

            if(next_idx == length){
               token_errorf("set '%s' ran into EOF while looking for ';'", curr, name->str);
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

            free(curr->str);
         }break;
         case EXPR_END:{
            if(stack_head == 0){
               token_error("';' Expects an expression before it", curr);
               error_status = 1;
               break;
            }

            if(last_was_op){
               token_errorf("Expected value instead got %s\n", curr, token_str(tokens[op_index - 1].type));
               error_status = 1;
               break;
            }

            free(curr->str);

            while(expr_size){
               program[*exe_len] = expression_stack[--expr_size];
               ++(*exe_len);
            }

            if(stack[stack_head - 1].type == SET_NUM){
               program[*exe_len] = stack[stack_head - 1];
               ++(*exe_len);

               stack_head--;
            }
            last_was_op = 1;
         }break;
         case COMMA:
            if(stack_size == 0){
               token_error("Comma in function call expects an expression before it", curr);
               error_status = 1;
               break;
            }

            if(last_was_op){
               token_errorf("Expected value instead got %s", curr, token_str(tokens[op_index - 1].type));
               error_status = 1;
               break;
            }

            free(curr->str);

            while(expr_size){
               if(expression_stack[expr_size - 1].type == NULL_TOKEN){
                  break;
               }
               program[*exe_len] = expression_stack[--expr_size];
               ++(*exe_len);
            }

            last_was_op = 1;
         break;
         case DEL: {
            if(op_index + 1 >= length || tokens[op_index + 1].type != ID){
               token_error("del expectes a variable name", curr);
            }

            char buffer[256];
            enum token_type id = NULL_TOKEN;
            L_TOKEN *name = &tokens[op_index + 1];
            sprintf(buffer, "%s_%s", get_function_prefix(stack, stack_head, in_function, ""), name->str);

            id = (enum token_type)map_get(symbols, buffer);
            if(id == NULL_TOKEN){
               id = (enum token_type)map_get(symbols, name->str);
            }else{ // variable is a function variable
               free(curr->str);
               curr->str = strdup(buffer);
            }

            if(id == NULL_TOKEN){
               token_error("Cannot delete Functions or ID's that have not been defined", curr);
               error_status = 1;
               break;
            }

            free(curr->str);
            map_delete_key(symbols, name->str);
            name->type = DEL;
            program[*exe_len] = conv_token(name);
            ++(*exe_len);

            op_index++;
            
         } break;
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
               token_errorf("ID: symbol[%s] is neither function nor variable", curr, curr->str);
               error_status = 1;
               break;
            }

            if(id == SET_NUM){
               program[*exe_len] = conv_token(curr);
               program[*exe_len].type = VAR_NUM;
               ++(*exe_len);
               stack_size++;
               last_was_op = 0;
            }else if(id == FUNCTION){
               char *function_name = curr->str;
               func_data *func_info = (func_data *)map_get(functions, function_name);
               if(func_info == NULL){
                  token_error("Unable to get function information", curr);
                  error_status = 1;
                  break;
               }
               if(op_index + 2 >= length){
                  token_errorf("While parsing call function %s ran into EOF", curr, curr->str);
                  error_status = 1;
                  break;
               }
               if(tokens[op_index + 1].type != PAREN_OPEN){
                  token_error("Function call expects '('", curr);
                  error_status = 1;
                  break;
               }
               free(tokens[op_index + 1].str);
               int args = 1;
               int nested_paren = 0;
               int p_o = 1;
               int p_c = 0;
               int j;
               for(j = 2; j + op_index < length; ++j){
                  if(tokens[op_index + j].type == PAREN_CLOSE){
                     // Look for the closing parenthese
                     nested_paren--;
                     p_c++;
                     if(nested_paren < 0){
                        free(tokens[op_index + j].str);
                        break;
                     }
                  }else if(tokens[op_index + j].type == PAREN_OPEN){
                     nested_paren++;
                     p_o++;
                  }else if(tokens[op_index + j].type == COMMA){
                     args++;
                  }
               }

               if(p_o != p_c){
                  // printf("%d:%d\n", p_o, p_c);
                  token_error("Parenthese missmatch in function call", curr);
                  error_status = 1;
                  break;
               }
         
               if(j == 2){
                  args = 0;
               }

               if(args != func_info->num_args){
                  token_errorf("%s expected %d arguments but got %d arguments", curr, curr->str, func_info->num_args, args);
                  error_status = 1;
                  break;
               }
               // Does removing this do anything
               //expression_stack[expr_size++] = (P_TOKEN){
               //   .type = NULL_TOKEN,
               //};

               tokens[op_index + j].type = CALL;
               tokens[op_index + j].str = strdup(function_name);
               free(curr->str);
               op_index += 1;
               last_was_op = 1;
            }else{
               // printf("%d\n", id);
               token_errorf("ID: Have not implemented id of type %s", curr, token_str(id));
               error_status = 1;
            }

         } break;
         case CALL: {
            if(stack_size == 0){
               token_error("Closing of function call expects an expression before it", curr);
               error_status = 1;
               break;
            }

            if(last_was_op){
               token_errorf("Expected value instead got %s\n", curr ,token_str(tokens[op_index - 1].type));
               error_status = 1;
               break;
            }

            while(expr_size){
               if(expression_stack[expr_size - 1].type == NULL_TOKEN){
                  break;
               }

               program[*exe_len] = expression_stack[--expr_size];
               ++(*exe_len);
            }

            last_was_op = 1;
     
            P_TOKEN ptkn = conv_token(curr);
            char *function_name = ptkn.name;
            ptkn.function = (func_data *)map_get(functions, ptkn.name);
            free(function_name);
            stack_size++;     
            last_was_op = 0;
            program[*exe_len] = ptkn;
            ++(*exe_len);
         } break;
         case FUNCTION:{
            if(op_index + 7 >= length){
               token_error("Incomplete function definiiton: function definition is required to use the following format\n"
                               "  func function_name(var1, ..., varN)\n"
                               "    ...\n"
                               "    return\n"
                               "  end", curr);
               error_status = 1;
               break;
            }

            L_TOKEN *func = &tokens[op_index + 1];
            if(func->type != ID || tokens[op_index + 2].type != PAREN_OPEN){
               token_error("Function definition is required to use the following format\n"
                               "  func function_name(var1, ..., varN)\n"
                               "    ...\n"
                               "    return\n"
                               "  end", curr);
               error_status = 1;
               break;
            }
            free(curr->str);
            free(tokens[op_index + 2].str);
            int j;
            int num_args = 0;
            char *arg_names[25];
            char arg_name_buffer[256];
            char *function_name = func->str;
            char *prefix = strdup(get_function_prefix(stack, stack_head, in_function, function_name));

            for(j = 3; op_index + j < length - 1; j += 2){
               L_TOKEN *var = &tokens[j + op_index];
               L_TOKEN *next = &tokens[j + op_index + 1];

               if(var->type != ID){
                  token_errorf("Function definition expects ID not %s", var, token_str(var->type));
                  error_status = 1;
                  goto ERR;
               }
               var->type = SET_NUM;
               sprintf(arg_name_buffer, "%s_%s", prefix, var->str);
               free(var->str);
               arg_names[num_args++] = strdup(arg_name_buffer);
               map_put(symbols, arg_name_buffer, (void *)SET_NUM);
               if(next->type == COMMA){
                  // There will be more variables
                  free(next->str);
               }else if(next->type == PAREN_CLOSE){
                  // There are no more variables
                  free(next->str);
                  break;
               }else{
                  token_errorf("Function definition expects ID not %s", next, token_str(var->type));
                  error_status = 1;
                  goto ERR;
               }
            }

            if(j + op_index + 1 >= length){
               token_error("Ran into End Of File while parsing function", curr);
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

            function_data->start = *exe_len;

            func->function_data = function_data;

            map_put(symbols, function_name, (void *)FUNCTION);
            map_put(functions, function_name, function_data);

            func->type = FUNCTION;
            P_TOKEN func_header = conv_token(func);
            func_header.function = function_data;
            stack[stack_head++] = func_header;
            program[*exe_len] = func_header;
            ++(*exe_len);

            op_index += j + 1;
         } break;
         case RETURN: {
            if(stack_size < 1){
               token_error("return needs a value\n", curr);
               error_status = 1;
               break;
            }

            program[*exe_len] = conv_token(curr);
            ++(*exe_len);
         } break;
         case END: {
            if(stack_head <= 0){
               token_error("END expects function definition", curr);
               error_status = 1;
               break;
            }
            if(op_index <= 0 || tokens[op_index - 1].type != RETURN){
               token_error("Last directive in function is ALWAYS return", &tokens[op_index - 1]);
               error_status = 1;
               break;
            }
            P_TOKEN func = stack[--stack_head];
            if(func.type != FUNCTION){
               token_errorf("Function ended in the middle of another statement. Got type %s", curr, token_str(func.type));
               error_status = 1;
               break;
            }
            free(curr->str);
            in_function--;
            func.function->end = *exe_len - 1;
            for(int j = 0; j < func.function->num_args; ++j){
               map_delete_key(symbols, func.function->args[j]);
            }
         } break;
         default:
            token_errorf("Cannot parse %s", curr, token_str(curr->type));
            exit(1);
            break;
      }
      // token_errorf("%s last_was_op = %d", curr, token_str(curr->type), last_was_op);
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
   map_destroy(functions);
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
      case NULL_TOKEN:
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
      case RETURN:
      case PAREN_OPEN:
      case PAREN_CLOSE:
         DEBUGF(3, "[MEM] free(%s)", token->str);
         free(token->str);
      break;
      case NUMBER:
         new_token.number = token->number;
      break;
      case SET_NUM:
      case CALL:
      case DEL:
      case ID:
         new_token.name = token->str;
      break;
      case FUNCTION:
         new_token.function = token->function_data;
      break;
      default:
         token_errorf("%s token conversion is not implemented yet\n", token, token_str(token->type));
         exit(1);
      break;
      
   }
   if(new_token.type == NULL_TOKEN){
      token_error("Made NULL_TOKEN", token);
      exit(1);
   }
   return new_token;
}

char *get_random_str(int size){
   static char buffer[256];
   if(size < 1 || size > 255){
      return "";
   }

   int num;

   for(int i = 0; i < size; ++i){
      num = rand() % 26;
      buffer[i] = 'A' + num;
   }

   buffer[size] = '\0';

   return buffer;
}
