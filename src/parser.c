#include <cutils/map.h>
#include <cutils/list.h>
#include <cutils/array.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <time.h>
#include <language.h>

void *assert_alloc(size_t size);

char *get_random_str(int size);

array_struct(s_array, char *);
array_struct(sa_array, s_array *);

char _sb_line_buffer[1024];

int s_array_contains(s_array *, char *);
void free_s_array(s_array *);

void start_scope(sa_array *scopes);
void end_scope(sa_array *scopes, map symbols, P_TOKEN *program, int *exe_len, int line);

int parse_program(L_TOKEN *tokens, int length, P_TOKEN *program, int *exe_len){
   int stack_size = 0;  
   int error_status = 0;
   int stack_head = 0;
   P_TOKEN stack[MAX_TOKENS];
   P_TOKEN expression_stack[MAX_TOKENS];
   sa_array scopes;
   array_init(&scopes, 10);

   int in_function = 0;
   int expr_size = 0;
   int last_was_op = 1;
   *exe_len = 0;
   map symbols = map_create(NULL);
   map functions = map_create(NULL);
   srand(142);

   for(int op_index = 0; op_index < length && !error_status; ++op_index){
      L_TOKEN *curr = &tokens[op_index];
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
            int scoped = 0;
            enum token_type id_type;
            name->str = id_search(name->str, stack, stack_head, in_function, symbols, &id_type);
            DEBUGF("found %s var", name->str);
            if(id_type != NULL_TOKEN){
               scoped = 1;
            }
            // if(in_function && !map_contains(symbols, name->str)){
            //    const char *prefix = get_function_prefix(stack, stack_head, in_function, "");
            //    sprintf(buffer, "%s_%s", prefix, name->str);
            //    free(name->str);
            //    name->str = strdup(buffer);
            // }else if(map_contains(symbols, name->str)){
            //    DEBUGF("%s is scoped", name->str);
            //    scoped = 1;
            // }

            // for(int i = 0; i < array_size(&scopes) && !scoped; ++i){
            //    s_array *current_scope = array_get(&scopes, i);
            //    for(int j = 0; j < array_size(current_scope) && !scoped; ++i){
            //       if(strcmp(name->str, array_get(current_scope, j)) == 0){
            //          DEBUGF("%s is already in a scope\n", name->str);
            //          scoped = 1;
            //       }
            //    }
            // }
            
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

            if(array_size(&scopes) > 0 && !scoped && !s_array_contains(array_get(&scopes, array_size(&scopes) - 1), name->str)){
               s_array *current_scope = array_get(&scopes, array_size(&scopes) - 1);
               array_append(current_scope, name->str);
            }

            if(next_idx == op_index + 1){
               program[*exe_len] = (P_TOKEN){
                  .type = NUMBER,
                  .number = 0,
                  .line = curr->line,
               };
               ++(*exe_len);
            }

            stack[stack_head] = conv_token(name);
            stack[stack_head].type = SET_NUM;
   
            stack_head++;
            last_was_op = 1;
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
            L_TOKEN *name = &tokens[op_index + 1];
            enum token_type id;

            name->str = id_search(name->str, stack, stack_head, in_function, symbols, &id);

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
            enum token_type id;
            curr->str = id_search(curr->str, stack, stack_head, in_function, symbols, &id);

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
                  token_error("Parenthese mismatch in function call", curr);
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
               DEBUGF("%s: %s", function_name, arg_name_buffer);
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

            // Init function data
            func_data *function_data = assert_alloc(sizeof(func_data));
            function_data->name = function_name;
            function_data->num_args = num_args;
            function_data->prefix = prefix;
            function_data->args = assert_alloc(num_args * sizeof(char *));

            // Init scope data
            start_scope(&scopes);

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
               token_error("END expects control flow item definition", curr);
               error_status = 1;
               break;
            }

            // Free variables scoped to the if statement
            end_scope(&scopes, symbols, program, exe_len, curr->line);
           
            P_TOKEN token = stack[--stack_head];
            if(token.type == FUNCTION){
               if(tokens[op_index - 1].type != RETURN){
                  token_error("Last directive in function is ALWAYS return", &tokens[op_index - 1]);
                  error_status = 1;
                  break;
               }
               in_function--;
               P_TOKEN ret = program[*exe_len - 1];
               program[*exe_len] = ret;
               ++(*exe_len);
               token.function->end = *exe_len - 1;
   
               for(int j = 0; j < token.function->num_args; ++j){
                  map_delete_key(symbols, token.function->args[j]);
               }
            }else if(token.type == IF_COND){
               // Set the end of the if body
               token.conditional->end = *exe_len - 1;
               token.conditional->next = *exe_len - 1;
            }else if(token.type == COND_END){
               token.conditional->end = *exe_len - 1;
               P_TOKEN curr_t;
               while(stack_head > 0){
                  curr_t = stack[stack_head - 1];

                  if(curr_t.type != COND_END && curr_t.type != IF_COND){
                     token_error("Else if statement mangled with other type of statement", curr);
                     error_status = 1;
                     goto ERR;
                  }else if(curr_t.type == COND_END){
                     curr_t.conditional->end = *exe_len - 1;     
                  }else{
                     curr_t.conditional->end = *exe_len - 1;     
                  }
                  stack_head--;
               }
            }else{
               token_errorf("end is not a supported token for type %s", curr, token_str(token.type));
               error_status = 1;
               break;
            }

            last_was_op = 1;
            free(curr->str);
         } break;
         case IF: {
            if(op_index + 3 >= length){
               token_error("if statement expects ( x )", curr);
               error_status = 1;
               break;
            }
            
            if(tokens[op_index + 1].type != PAREN_OPEN){
               token_errorf("if statments expects '(' not %s", curr, token_str(curr->type));
               error_status = 1;
               break;
            }
   
            if(stack_head > 0 && stack[stack_head - 1].type == COND_END){
               end_scope(&scopes, symbols, program, exe_len, curr->line);
            }

            int nested_paren = 0;
            int p_o = 1;
            int p_c = 0;
            int j;
            for(j = 2; j + op_index < length; ++j){
               enum token_type current_type = tokens[op_index + j].type;
               if(current_type == PAREN_OPEN){
                  nested_paren++;
                  p_o++;
               }else if(current_type == PAREN_CLOSE){
                  p_c++;
                  nested_paren--;
                  if(nested_paren < 0){
                     free(tokens[op_index + j].str);
                     break;
                  }
               }
            }

            if(p_o != p_c){
               token_error("Parenthese mismatch in if statement", curr);
               error_status = 1;
               break;
            }

            if(j == 2){
               token_error("if statement with empty conditional", curr);
               error_status = 1;
               break;
            }

            free(curr->str);
            cond_data *cond_info = assert_alloc(sizeof(cond_data));
            cond_info->start = *exe_len;
            cond_info->end = -1;
            cond_info->next = -1;

            free(tokens[op_index + 1].str);
            tokens[op_index + j].type = IF_COND;
            tokens[op_index + j].cond_info = cond_info;

            // Init scope data
            start_scope(&scopes);

            op_index += 1;
            last_was_op = 1;
         } break;
         case ELSE: {
            if(stack_head <= 0 || stack[stack_head - 1].type != IF_COND){
               token_error("else statement expects if statement", curr);
               error_status = 1;
               break;
            }

            free(curr->str);
            // Free variables scoped to the if statement
            end_scope(&scopes, symbols, program, exe_len, curr->line);

            // Init scope data
            start_scope(&scopes);

            stack[stack_head - 1].conditional->next = *exe_len;
            stack[stack_head - 1].type = COND_END;
            program[*exe_len] = stack[stack_head - 1];
            ++(*exe_len);
            last_was_op = 1;
         } break;
         case IF_COND: {
            stack[stack_head] = conv_token(curr);
            program[*exe_len] = stack[stack_head++];
            ++(*exe_len);
            last_was_op = 1;
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
   array_delete(&scopes);
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
      .line = token->line,
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
         DEBUGF( "[MEM] free(%s)", token->str);
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
      case IF_COND:
         new_token.conditional = token->cond_info;
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
   if(size < 1){
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

char *id_search(char *name, P_TOKEN *stack, int stack_head, int in_func, map env, enum token_type *id){
   char *result = name;
   char buffer[256];
   *id = NULL_TOKEN;
   sprintf(buffer, "%s_%s", get_function_prefix(stack, stack_head, in_func, ""), name);

   DEBUGF("looking for %s", buffer);

   *id = (enum token_type)map_get(env, buffer);
   if(*id == NULL_TOKEN){
      *id = (enum token_type)map_get(env, name);
   }else{ // variable is a function variable
      free(name);
      result = strdup(buffer);
   }

   return result;
}

int s_array_contains(s_array *arr, char *s){
   for(int i = 0; i < array_size(arr); ++i){
      if(strcmp(array_get(arr, i), s) == 0){
         return 1;
      }
   }

   return 0;
}

void free_s_array(s_array *arr){
   free(arr->array);
   free(arr);
}

void start_scope(sa_array *scopes){
   s_array *local_scope = assert_alloc(sizeof(s_array));
   array_init(local_scope, 10);
   array_append(scopes, local_scope);
}

void end_scope(sa_array *scopes, map symbols, P_TOKEN *program, int *exe_len, int line){
   s_array *current_scope = array_get(scopes, array_size(scopes) - 1);
   for(int i = 0; i < array_size(current_scope); ++i){
      program[*exe_len] = (P_TOKEN){
         .type = DEL,
         .name = array_get(current_scope, i),
         .line = line,
      };
      DEBUGF("Removing %s from scope", array_get(current_scope, i));
      map_delete_key(symbols, array_get(current_scope, i));
      ++(*exe_len);
   }
   array_remove(scopes, array_size(scopes) - 1, free_s_array);
}
