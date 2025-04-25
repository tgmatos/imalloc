#include "imalloc.h"

size_t imemalignment(void const* p)
{
    intptr_t ip = (intptr_t)p;
    return ip & -ip; // convert to LSB value
}

imonotonic_heap* imonotonic_heap_init(void* mem, size_t size)
{
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
