INC=-I$(RAYLIB_INCLUDE) -I./include/
LIBS= -lraylib -lGL -lm -pthread -ldl
CFLAGS=-Wall -Wextra
SRCS=src/main.c src/flow.c src/utils.c src/panel.c src/globals.c src/button.c src/options.c src/prints.c

all: ctrlf

ctrlf: $(SRCS)
	gcc $(CFLAGS) $(INC) -L. $(SRCS) -o ctrlf $(LIBS)

clean:
	rm dflow
