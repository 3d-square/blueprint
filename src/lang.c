#include <cutils/map.h>
#include <cutils/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <language.h>

int main(int argc, char **argv){
   P_TOKEN program[MAX_TOKENS];
   int size;

   if(argc < 2){
      fprintf(stderr, "usage: %s [file] ...\n", argv[0]);
      return 1;
   }

   for(int i = 1; i < argc; ++i){
      printf("Running file: %s\n", argv[i]);
      if((size = read_parse_file(argv[i], program)) != -1){
         run_program(program, size);
      }else{
         fprintf(stderr, "Parses exited with errors\n");
         return 1;
      }
   }

   return 0;
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


