#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct imonotonic_heap
{
    void* free_pointer;
    size_t freesize;
    _Alignas(max_align_t) char area[0];
} imonotonic_heap;

size_t imemalignment(void const* p);
imonotonic_heap* imonotonic_heap_init(void* mem, size_t size);
void* imonotonic_malloc(imonotonic_heap* heap, size_t size);
