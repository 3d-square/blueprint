#pragma once

typedef struct {
   size_t allocated;
   size_t freed;
} TMALLOC_INFO;
#define tmalloc(sz) tcalloc(1, sz);
void *tcalloc(size_t, size_t);
void tfree(void *, size_t);

const TMALLOC_INFO *get_tmalloc_info(void);
void report_tmalloc_info(void);

#ifdef INIT_TMALLOC

static TMALLOC_INFO _tmalloc_info = {0};

void *tcalloc(size_t count, size_t size){
   _tmalloc_info.allocated += count * size;

   return calloc(count, size);
}

void tfree(void *ptr, size_t size){
   _tmalloc_info.freed -= size;

   free(ptr);
}

const TMALLOC_INFO *get_tmalloc_info(void){
   return &_tmalloc_info;
}
void report_tmalloc_info(void){
   printf("MALLOC_INFO:\n"
          "  allocated: %lu\n"
          "  freed: %lu\n"
   )
}

#endif
