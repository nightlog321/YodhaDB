CC=gcc
CFLAGS=-Wall -O3 -mavx2 -Iinclude
LIBS=-lsqlite3

CORE_SRCS = \
	src/rowgroup.c \
	src/scan.c \
	src/scan_simd.c \
	src/ai_shim.c

all: test_rg bench ai_cli

test_rg:
	$(CC) $(CFLAGS) $(CORE_SRCS) tests/test_rowgroup.c -o test_rg

bench:
	$(CC) $(CFLAGS) $(CORE_SRCS) tests/benchmark_sqlite_vs_yodha.c -o bench $(LIBS)

ai_cli:
	$(CC) $(CFLAGS) $(CORE_SRCS) tools/ai_cli.c -o ai_cli

clean:
	rm -f test_rg bench ai_cli *.ydb *.sqlite