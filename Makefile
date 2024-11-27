# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -DDEBUG

# Target for testing the custom heap manager implementation
test_heap_manager: test_heap_manager.c heap_manager.c heap_manager.h
	$(CC) $(CFLAGS) -DUSE_CUSTOM -o test_heap_manager test_heap_manager.c heap_manager.c -lpthread

# Target for testing the system's malloc/free/realloc
system_test: test_heap_manager.c
	$(CC) $(CFLAGS) -o system_test test_heap_manager.c -lpthread

# Clean up generated files
clean:
	rm -f test_heap_manager system_test
