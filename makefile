INC=-I$(RAYLIB_INCLUDE) -I./include/
LIBS= -lraylib -lGL -lm -pthread -ldl
CFLAGS=-Wall -Wextra
SRCS=src/main.c src/flow.c src/utils.c src/panel.c src/globals.c src/button.c

all: dflow

dflow: $(SRCS)
	gcc $(CFLAGS) $(INC) -L. $(SRCS) -o dflow $(LIBS)

clean:
	rm dflow
