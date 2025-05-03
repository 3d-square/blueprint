INC=-I$(RAYLIB_INCLUDE) -I./include/
LIBS= -lraylib -lGL -lm -pthread -ldl -lcutils
LPATH=-L$(HOME)/cutils/bin/ -L.
RPATH=-Wl,-rpath,$(HOME)/cutils/bin/
CFLAGS=-Wall -Wextra
DEFS=
SRCS=src/main.c src/flow.c src/utils.c src/panel.c src/globals.c src/prints.c src/ui.c src/directory_info.c src/filewin.c
LANG_SRCS=src/lang.c src/parser.c src/interpreter.c src/lexer.c

all: ctrlf lang

ctrlf: $(SRCS)
	gcc $(CFLAGS) $(INC) $(LPATH) $(DEFS) $(RPATH) $(SRCS) -o ctrlf $(LIBS)

lang: $(LANG_SRCS)
	@echo $(DEFS)
	gcc $(CFLAGS) $(INC) $(LPATH) $(DEFS) -DDEBUG_MSG $(RPATH) $(LANG_SRCS) -o lang-debug  $(LIBS)
	gcc $(CFLAGS) $(INC) $(LPATH) $(DEFS) $(RPATH) $(LANG_SRCS) -o lang  $(LIBS)

clean:
	rm lang
	rm ctrlf
