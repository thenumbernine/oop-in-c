CFLAGS = -Wall 
#CFLAGS += -std=c89

.PHONY: all
all: main

main.o: main.c
	gcc -c -o main.o ${CFLAGS} main.c

main: main.o
	gcc -o main -pthread main.o

.PHONY: clean
clean:
	-rm main.o
	-rm main
