#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <language.h>

sb *file_lines = NULL;
size_t num_lines = 0;

int main(int argc, char **argv){
   P_TOKEN program[MAX_TOKENS];
   int size;

   if(argc < 2){
      fprintf(stderr, "usage: %s [file] ... <debug>\n", argv[0]);
      return 1;
   }

   int i = 1;
   int debug = 0;
   if(strncmp(argv[1], "-d=", 3) == 0){
      if(strcmp(argv[1] + 3, "y") == 0 || strcmp(argv[1] + 3, "yes") == 0){
         debug = 1;
      }
      i++;
   }

   for(; i < argc; ++i){
      printf("Running file: %s\n", argv[i]);
      if((size = read_parse_file(argv[i], program)) != -1){
         run_program(program, size, debug);
      }else{
         fprintf(stderr, "Parses exited with errors\n");
         return 1;
      }
      sb_free_list(file_lines, num_lines);
   }

   return 0;
}


int read_parse_file(const char *file_name, P_TOKEN *program){
   DEBUG("reading file");
   L_TOKEN tokens[MAX_TOKENS];
   int index = 0;
   int program_size = 0;
   lexer_from_file(file_name);

   while(!lexer_is_empty()){
      tokens[index] = read_token();
      if(tokens[index].type == NONE) break;
      DEBUGF("read: %s", token_str(tokens[index].type));
      index++;
   }

   DEBUGF( "[PROGRAM] Num Tokens: %d", index);
   lexer_destroy();

   file_lines = sb_read_lines(file_name, &num_lines);
   
   if(parse_program(tokens, index, program, &program_size)){
      fprintf(stderr, "Parsing failed\n");
      return -1;
   }

   DEBUGF( "[PROGRAM] Program size: %d", program_size);
   return program_size;
}


