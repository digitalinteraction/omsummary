BIN_NAME = omsummary
CC = gcc
CFLAGS = -O3 -Wall -march=native
LIBS = -lm

SRC = $(wildcard *.c)
INC = $(wildcard *.h)

all: $(BIN_NAME)

$(BIN_NAME): Makefile $(SRC) $(INC)
	$(CC) -std=c99 -o $(BIN_NAME) $(CFLAGS) $(SRC) -I/usr/local/include -L/usr/local/lib $(LIBS)

clean:
	rm -f *.o core $(BIN_NAME)
