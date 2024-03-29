#CC = clang
CC = gcc
CFLAGS = -g -O0 -Wall 
#CFLAGS += -std=c89
#CFLAGS += -std=c18
#CFLAGS += -std=gnu18

.PHONY: all
all: main

main.o: main.c *.h
	$(CC) -c -o main.o $(CFLAGS) main.c

main.i: main.c *.h
	$(CC) -c -o main.i $(CFLAGS) -E main.c

# find where the preprocessor is going wrong
# by forcing a preprocessor output and then compiling it
main.i.o: main.i
	$(CC) -c -o main.i.o $(CFLAGS) main.i

main: main.o
	$(CC) -o main -pthread main.o

.PHONY: clean
clean:
	-rm main.o
	-rm main
