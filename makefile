INC=-I$(RAYLIB_INCLUDE) -I./include/
LIBS= -lraylib -lGL -lm -pthread -ldl
CFLAGS=-Wall -Wextra
SRCS=src/main.c src/flow.c src/utils.c src/panel.c src/globals.c src/prints.c src/ui.c src/directory_info.c src/filewin.c

all: ctrlf lang

ctrlf: $(SRCS)
	gcc $(CFLAGS) $(INC) -L. $(SRCS) -o ctrlf $(LIBS)

lang: src/lang.c
	gcc $(CFLAGS) $(INC) -L. src/lang.c src/l_hash.c -o lang 

clean:
	rm dflow
