CC=gcc
CFLAGS= -Wall -Werror -pedantic -g3 -fsanitize=address,undefined

all: mnem2op op2mnem

mnem2op: mnem2op.c
	$(CC) $(CFLAGS) $^ -o $@

op2mnem: op2mnem.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f mnem2op op2mnem
