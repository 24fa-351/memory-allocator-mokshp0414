#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "heap_manager.h"

#ifdef DEBUG
#define LOG(msg, ...) fprintf(stderr, "[LOG] " msg "\n", ##__VA_ARGS__)
#else
#define LOG(msg, ...)
#endif

#define ERROR(msg, ...) fprintf(stderr, "[ERROR] " msg "\n", ##__VA_ARGS__)

typedef struct block_header
{
    size_t size;
    int free;
    struct block_header *next;
} block_header_t;

#define HEAP_CAPACITY 1024
typedef struct min_heap
{
    block_header_t *data[HEAP_CAPACITY];
    size_t size;
} min_heap_t;

// Globals
static void *heap_start = NULL;
static size_t heap_size = 0;
static min_heap_t free_blocks;
static pthread_mutex_t heap_lock = PTHREAD_MUTEX_INITIALIZER;

#define ALIGN4(size) (((size) + 3) & ~3)
#define HEADER_SIZE (sizeof(block_header_t))

void heap_init(min_heap_t *heap)
{
    heap->size = 0;
}

void heap_insert(min_heap_t *heap, block_header_t *block)
{
    if (heap->size >= HEAP_CAPACITY)
    {
        ERROR("Heap capacity exceeded");
        return;
    }
    size_t i = heap->size++;
    while (i > 0)
    {
        size_t parent = (i - 1) / 2;
        if (heap->data[parent]->size <= block->size)
            break;
        heap->data[i] = heap->data[parent];
        i = parent;
    }
    heap->data[i] = block;
}

block_header_t *heap_extract_min(min_heap_t *heap)
{
    if (heap->size == 0)
        return NULL;
    block_header_t *min = heap->data[0];
    block_header_t *last = heap->data[--heap->size];
    size_t i = 0;
    while (2 * i + 1 < heap->size)
    {
        size_t left = 2 * i + 1;
        size_t right = left + 1;
        size_t smallest = (right < heap->size && heap->data[right]->size < heap->data[left]->size) ? right : left;
        if (last->size <= heap->data[smallest]->size)
            break;
        heap->data[i] = heap->data[smallest];
        i = smallest;
    }
    heap->data[i] = last;
    return min;
}

void init_heap(size_t size)
{
    pthread_mutex_lock(&heap_lock);
    heap_start = malloc(size);
    if (!heap_start)
    {
        ERROR("Failed to initialize heap");
        pthread_mutex_unlock(&heap_lock);
        exit(1);
    }
    heap_size = size;
    block_header_t *initial_block = (block_header_t *)heap_start;
    initial_block->size = size - HEADER_SIZE;
    initial_block->free = 1;
    initial_block->next = NULL;
    heap_init(&free_blocks);
    heap_insert(&free_blocks, initial_block);
    LOG("Heap initialized with size %zu bytes", size);
    pthread_mutex_unlock(&heap_lock);
}

void *my_malloc(size_t size)
{
    pthread_mutex_lock(&heap_lock);
    size = ALIGN4(size);
    block_header_t *block = heap_extract_min(&free_blocks);
    if (!block || block->size < size)
    {
        ERROR("No suitable block found for allocation of size %zu", size);
        pthread_mutex_unlock(&heap_lock);
        return NULL;
    }
    if (block->size > size + HEADER_SIZE)
    {
        block_header_t *new_block = (block_header_t *)((char *)block + HEADER_SIZE + size);
        new_block->size = block->size - size - HEADER_SIZE;
        new_block->free = 1;
        new_block->next = NULL;
        heap_insert(&free_blocks, new_block);
        block->size = size;
    }
    block->free = 0;
    void *allocated_mem = (char *)block + HEADER_SIZE;
    memset(allocated_mem, 0, size);
    LOG("Allocated %zu bytes at address %p", size, allocated_mem);
    pthread_mutex_unlock(&heap_lock);
    return allocated_mem;
}

void my_free(void *ptr)
{
    if (!ptr)
        return;
    pthread_mutex_lock(&heap_lock);
    block_header_t *block = (block_header_t *)((char *)ptr - HEADER_SIZE);
    block->free = 1;
    heap_insert(&free_blocks, block);
    LOG("Freed block at address %p", ptr);
    pthread_mutex_unlock(&heap_lock);
}

void *my_realloc(void *ptr, size_t size)
{
    if (!ptr)
        return my_malloc(size);
    if (size == 0)
    {
        my_free(ptr);
        return NULL;
    }
    block_header_t *block = (block_header_t *)((char *)ptr - HEADER_SIZE);
    if (block->size >= size)
        return ptr;
    void *new_ptr = my_malloc(size);
    if (new_ptr)
    {
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
    }
    LOG("Reallocated block from %p to %p with size %zu bytes", ptr, new_ptr, size);
    return new_ptr;
}
