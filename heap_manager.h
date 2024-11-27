#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stddef.h>

// Initialize the heap
void init_heap(size_t size);

// Allocate memory
void* my_malloc(size_t size);

// Free memory
void my_free(void* ptr);

// Reallocate memory
void* my_realloc(void* ptr, size_t size);

#endif // HEAP_MANAGER_H
