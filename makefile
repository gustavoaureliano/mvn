CC=gcc
CFLAGS= -Wall -Werror -pedantic -g3 -fsanitize=address,undefined

.PHONY: all clean test test-asm test-roundtrip test-ref test-monitor

all: mnem2op op2mnem

mnem2op: mnem2op.c
	$(CC) $(CFLAGS) $^ -o $@

op2mnem: op2mnem.c
	$(CC) $(CFLAGS) $^ -o $@

test: all test-asm test-roundtrip

test-asm: all
	./scripts/test_asm.sh

test-roundtrip: all
	./scripts/test_roundtrip.sh

test-ref: all
	./scripts/test_ref.sh

test-monitor: all
	./scripts/test_monitor.sh

clean:
	rm -f mnem2op op2mnem
