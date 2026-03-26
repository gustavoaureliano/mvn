CC=gcc
CFLAGS= -Wall -Werror -pedantic -g -fsanitize=address,undefined

all: mnem2op

mnem2op: mnem2op.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm mnem2op

