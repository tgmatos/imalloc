#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct imonotonic_heap
{
    void* free_pointer;
    size_t freesize;
    _Alignas(max_align_t) char area[0];
} imonotonic_heap;

size_t imemalignment(void const* p);
imonotonic_heap* imonotonic_heap_init(void* mem, size_t size);
void* imonotonic_malloc(imonotonic_heap* heap, size_t size);

typedef struct ibuddy_block
{
    size_t size;
    bool used;
    _Alignas(max_align_t) char area[0];
} ibuddy_block;

typedef struct ibuddy_heap
{
    ibuddy_block* head;
    ibuddy_block* tail;
    size_t size;
    _Alignas(max_align_t) char area[0];
} ibuddy_heap;

/*
  Note: The size must be of at least 16 bytes since
  and size - sizeof(ibuddy_leaf) must be a power of 2
  So if you want to allocate 256 KB
  you must allocate 256KB + sizeof(ibuddy_leaf).
*/
ibuddy_heap* ibuddy_heap_init(void* mem, size_t size);
void* ibuddy_malloc(ibuddy_heap* heap, size_t size);
void* ibuddy_malloc_first(ibuddy_heap* heap, size_t size);
