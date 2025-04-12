INC=-I$(RAYLIB_INCLUDE) -I./include/
LIBS= -lraylib -lGL -lm -pthread -ldl -lcutils
LPATH=-L$(HOME)/cutils/bin/ -L.
RPATH=-Wl,-rpath,$(HOME)/cutils/bin/
CFLAGS=-Wall -Wextra
DEFS=
SRCS=src/main.c src/flow.c src/utils.c src/panel.c src/globals.c src/prints.c src/ui.c src/directory_info.c src/filewin.c

DEFS +=-DDEBUG_MSG

all: ctrlf lang

ctrlf: $(SRCS)
	gcc $(CFLAGS) $(INC) $(LPATH) $(DEFS) $(RPATH) $(SRCS) -o ctrlf $(LIBS)

lang: src/lang.c src/lexer.c
	@echo $(DEFS)
	gcc $(CFLAGS) $(INC) $(LPATH) $(DEFS) $(RPATH) src/lang.c src/lexer.c -o lang  $(LIBS)

clean:
	rm lang
	rm ctrlf
