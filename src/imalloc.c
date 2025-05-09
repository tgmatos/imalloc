#include "imalloc.h"

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
    size_t sz = size - sizeof(ibuddy_heap);
    assert((sz & (sz - 1)) == 0);
    assert((sz > sizeof(ibuddy_block) + sizeof(ibuddy_heap)) && "Size is less than the size of the block");
    memset(mem, 0, sz);

    ibuddy_heap* heap = (ibuddy_heap*)((char*)mem);
    ibuddy_block* block = (ibuddy_block*)((char*)mem + sizeof(ibuddy_heap));

    block->size = sz;
    block->used = false;

    heap->head = block;

    heap->tail = (ibuddy_block*)((char*)block + block->size);

    heap->size = sz;
    return heap;
}

void* ibuddy_malloc_first(ibuddy_heap* heap, size_t size)
{
    assert((size & (size - 1)) == 0);
    assert(size > 0);

    ibuddy_block* mem = heap->head;
    mem->size = heap->size;
    size_t sz = mem->size;
    ibuddy_block* buddy;

    while (mem < heap->tail && mem->size > size)
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
        return (char*)mem + sizeof(ibuddy_block);
    }

    return NULL;
}

// MIN_LEN MUST BE OF THE SIZE OF THE BUDDY_BLOCK STRUCT
void* ibuddy_malloc(ibuddy_heap* heap, size_t size)
{
    assert((size > 0) && "Size is less than or equal to zero");
    ibuddy_block* mem = heap->head;
    ibuddy_block* buddy = (ibuddy_block*)((char*)mem + mem->size);

    size_t sz = mem->size;
    size = size + sizeof(ibuddy_block);

    size_t power = 1;
    while (power < size)
    {
        power <<= 1;
    }

    if (buddy == heap->tail)
    {
        return ibuddy_malloc_first(heap, power);
    }

    while (mem < heap->tail)
    {
        mem = (ibuddy_block*)((char*)mem + mem->size);
        if (mem->used == false && power == mem->size)
        {
            mem->used = true;
            return (char*)mem + sizeof(ibuddy_block);
        }

        if (mem->used == false && power < mem->size)
        {
            sz = mem->size;
            mem->size = sz / 2;
            mem->used = false;
            buddy = (ibuddy_block*)((char*)mem + mem->size);
            buddy->size = sz / 2;
            buddy->used = false;
            sz = mem->size;

            if (power == mem->size)
            {
                mem->used = true;
                mem->size = power;
                return (char*)mem + sizeof(ibuddy_block);
            }
        }
    }
    return NULL;
}

void ibuddy_free_sized(ibuddy_heap* heap, void* mem, size_t size)
{
    if (mem == NULL)
    {
        return;
    }
    size_t sz;

    ibuddy_block* block = (ibuddy_block*)((char*)mem - sizeof(ibuddy_block));
    block->used = false;

    block = heap->head;
    ibuddy_block* buddy = (ibuddy_block*)((char*)block + block->size);

    while (block < heap->tail)
    {
        if ((block->used == false && buddy->used == false) && (block->size == buddy->size))
        {
            sz = block->size * 2;
            memset((char*)block, 0, sz);
            block->size = sz;
            block->used = false;
            block = (ibuddy_block*)((char*)block + block->size);
            buddy = (ibuddy_block*)((char*)block + block->size);
        }
        else
        {
            block = (ibuddy_block*)((char*)buddy + buddy->size);
            buddy = (ibuddy_block*)((char*)block + block->size);
        }
        if (buddy >= heap->tail)
        {
            return;
        }
    }
}

void* ibuddy_realloc(ibuddy_heap* heap, void* mem, size_t size)
{
    if (mem == NULL)
    {
        return ibuddy_malloc(heap, size);
    }

    ibuddy_block* b = (ibuddy_block*)((char*)mem - sizeof(ibuddy_block));

    if (b->size > size)
    {
        return (char*)mem;
    }

    void* block = ibuddy_malloc(heap, size);
    if (block == NULL)
    {
        b->used = true;
        return NULL;
    }
    b->used = false;

    memcpy(block, mem, b->size - sizeof(ibuddy_block));
    return (char*)block;
}
