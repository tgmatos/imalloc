#include "imalloc.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

ibuddy_heap* ibuddy_heap_init(void* mem, size_t size)
{

    assert((size & (size - 1)) == 0);
    assert((size > sizeof(ibuddy_block) + sizeof(ibuddy_heap)) && "Size is less than the size of the block");
    memset(mem, 0, size);

    ibuddy_heap* heap = (ibuddy_heap*)mem;
    heap->head = mem + sizeof(ibuddy_heap);
    heap->tail = mem + sizeof(ibuddy_heap);
    heap->size = size;
    return heap;
}

void* ibuddy_malloc_first(ibuddy_heap* heap, size_t size)
{
    assert((size & (size - 1)) == 0);
    assert(size > 0);
    ibuddy_block* mem = heap->head;
    mem->size = heap->size;
    size_t sz = mem->size;
    size_t sz2 = mem->size;
    ibuddy_block* buddy = NULL;

    while (size < mem->size)
    {
        mem->size = sz / 2;
        mem->used = false;
        buddy = (ibuddy_block*)((char*)mem + mem->size);
        buddy->size = sz / 2;
        buddy->used = false;
        sz = mem->size;
    }

    if (mem->size == size)
    {
        mem->used = true;
        mem->size = size;
        heap->tail = buddy;
        return ((char*)mem + sizeof(ibuddy_block));
    }

    return NULL;
}

// MIN_LEN MUST BE OF THE SIZE OF THE BUDDY_BLOCK STRUCT
void* ibuddy_malloc(ibuddy_heap* heap, size_t size)
{
    assert((size > 0) && "Size is less than or equal to zero");
    assert((size > sizeof(ibuddy_block)) && "Size is less than the size of the block");

    ibuddy_block* mem = heap->head;
    ibuddy_block* buddy = (ibuddy_block*)((char*)mem + mem->size);
    size_t sz = mem->size;

    uint32_t power = 1;
    while (power < size)
    {
        power <<= 1;
    }

    if (heap->head == heap->tail)
    {
        return ibuddy_malloc_first(heap, power);
    }

    if (heap->tail->used == false && power == heap->tail->size)
    {
        heap->tail->used = true;
        return (char*)heap->tail + sizeof(ibuddy_block);
    }

    while (mem->size < heap->size && mem->size > 0)
    {
        mem = (ibuddy_block*)((char*)mem + mem->size);

        if (mem->used == false && power == mem->size)
        {
            mem->used = true;
            heap->tail = (ibuddy_block*)((char*)mem + mem->size);
            return (char*)mem + sizeof(ibuddy_block);
        }

        if (mem->used == false && power < mem->size)
        {
            sz = mem->size;
            while (power < mem->size)
            {
                mem->size = sz / 2;
                mem->used = false;
                buddy = (ibuddy_block*)((char*)mem + mem->size);
                buddy->size = sz / 2;
                buddy->used = false;
                sz = mem->size;
            }

            if (power == mem->size)
            {
                mem->used = true;
                mem->size = power;
                heap->tail = (ibuddy_block*)((char*)mem + mem->size);
                return (char*)mem + sizeof(ibuddy_block);
            }
        }
    }

    return NULL;
}

void ibuddy_free_sized(ibuddy_heap* heap, ibuddy_block* mem, size_t size)
{
    ibuddy_block* buddy;
    ibuddy_block* block = (ibuddy_block*)((char*)mem - sizeof(ibuddy_block));
    block->used = false;

    // coalesce blocks
    block = heap->head;
    buddy = (ibuddy_block*)((char*)block + block->size);

    while (block->size * 2 < heap->size)
    {
        if (block->used == false && buddy->used == false && block->size == buddy->size)
        {
            block->size = block->size * 2;
            memset(block + sizeof(ibuddy_block), 0, block->size - sizeof(ibuddy_block));
        }

        block = (ibuddy_block*)((char*)block + block->size);
        if (block->size * 2 < heap->size)
        {
            buddy = (ibuddy_block*)((char*)block + block->size);
        }
    }
}

void* ibuddy_realloc(ibuddy_heap* heap, void* mem, size_t size)
{
    ibuddy_block* b = (ibuddy_block*)((char*)mem - sizeof(ibuddy_block));
    uint32_t power = 1;
    while (power < size)
    {
        power <<= 1;
    }

    b->used = false;
    void* block = ibuddy_malloc(heap, power);
    if (block == NULL)
    {
        b->used = true;
        return NULL;
    }

    memcpy(block, mem, b->size);
    return (char*)block;
}
