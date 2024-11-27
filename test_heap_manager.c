#include <stdio.h>
#include <stdlib.h>
#include "heap_manager.h"

#ifdef USE_CUSTOM
#define malloc(size) my_malloc(size)
#define free(ptr) my_free(ptr)
#define realloc(ptr, size) my_realloc(ptr, size)
#endif

void test_malloc_free()
{
    printf("Running test_malloc_free...\n");
    void *ptr1 = malloc(128);
    void *ptr2 = malloc(256);
    if (!ptr1 || !ptr2)
    {
        printf("test_malloc_free: FAILED (malloc returned NULL)\n");
        return;
    }
    free(ptr1);
    free(ptr2);
    printf("test_malloc_free: PASSED\n");
}

void test_realloc()
{
    printf("Running test_realloc...\n");
    void *ptr = malloc(128);
    if (!ptr)
    {
        printf("test_realloc: FAILED (malloc returned NULL)\n");
        return;
    }
    ptr = realloc(ptr, 256);
    if (!ptr)
    {
        printf("test_realloc: FAILED (realloc returned NULL)\n");
        return;
    }
    free(ptr);
    printf("test_realloc: PASSED\n");
}

int main()
{
#ifdef USE_CUSTOM
    printf("Initializing custom heap...\n");
    init_heap(1024);
#endif

    test_malloc_free();
    test_realloc();

    return 0;
}
