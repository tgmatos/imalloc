#include "imalloc.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

size_t imemalignment(void const* p)
{
    intptr_t ip = (intptr_t)p;
    return ip & -ip; // convert to LSB value
}

imonotonic_heap* imonotonic_heap_init(void* mem, size_t size)
{
    assert(size > sizeof(imonotonic_heap));
    assert(imemalignment(mem) >= _Alignof(imonotonic_heap));
    imonotonic_heap* heap = (imonotonic_heap*)mem;
    heap->free_pointer = (char*)mem + offsetof(imonotonic_heap, area);
    heap->freesize = size - sizeof(imonotonic_heap);
    return heap;
}

void* imonotonic_malloc(imonotonic_heap* heap, size_t size)
{
    if (heap->freesize < size)
    {
        return NULL;
    }

    heap->freesize -= size;
    void* pointer = heap->free_pointer;
    size_t remainder = _Alignof(max_align_t) % size;
    size_t padding = size - remainder;
    size += padding;
    heap->free_pointer = (char*)heap->free_pointer + size;

    return pointer;
}

// Should I count the size of the struct on the memory? IDK.
// Must divide all memory on the init
ibuddy_heap* ibuddy_heap_init(void* mem, size_t size)
{
    assert((size & (size - 1)) == 0);

    ibuddy_heap* heap = (ibuddy_heap*)mem;
    heap->head = mem + sizeof(ibuddy_heap);
    heap->tail = mem + sizeof(ibuddy_heap);
    heap->size = size;
    return heap;
}

// Note: The size passed must be already rounded
void* ibuddy_malloc_first(ibuddy_heap* heap, size_t size)
{
    assert((size & (size - 1)) == 0);
    assert(size > 0);
    ibuddy_block* mem = heap->head;
    mem->size = heap->size;
    size_t sz = mem->size;
    ibuddy_block* buddy = NULL;
    /* printf("Mem size: %zu - Size: %zu\n", mem->size, size); */

    // Case for when the max block is not being used
    // So I must split it until I find the block I want and return it
    printf("Address heap: %p - Address mem: %p\n", (void*)heap, (void*)mem);

    while (size < mem->size && size <= sizeof(ibuddy_block))
    {
        /* printf("mem->size: %zu - Address mem: %p\n", mem->size, mem); */
        mem->size = sz / 2;
        mem->used = false;
        // Get the the buddy block
        buddy = (ibuddy_block*)((char*)mem + mem->size);
        buddy->size = sz / 2;
        buddy->used = false;
        sz = mem->size;
    }

    if (mem->size == size)
    {
        mem->used = true;
        mem->size = size;
        heap->tail = mem + mem->size;
        return (char*)mem + sizeof(ibuddy_block);
    }

    return NULL;
}

// MIN_LEN MUST BE OF THE SIZE OF THE BUDDY_BLOCK STRUCT
void* ibuddy_malloc(ibuddy_heap* heap, size_t size)
{
    assert(size > 0);

    uint32_t power = 1;
    while (power < size)
    {
        power <<= 1;
    }

    // Case where the I do the first allocation
    if (heap->head == heap->tail)
    {
        return ibuddy_malloc_first(heap, power);
    }

    return NULL;
}
