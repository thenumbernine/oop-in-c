CFLAGS = -Wall 
#CFLAGS += -std=c89

.PHONY: all
all: main

main.o: main.c
	gcc -c -o main.o $(CFLAGS) main.c

main.i: main.c
	gcc -c -o main.i $(CFLAGS) -E main.c

# find where the preprocessor is going wrong
# by forcing a preprocessor output and then compiling it
main.i.o: main.i
	gcc -c -o main.i.o $(CFLAGS) main.i

main: main.o
	gcc -o main -pthread main.o

.PHONY: clean
clean:
	-rm main.o
	-rm main
