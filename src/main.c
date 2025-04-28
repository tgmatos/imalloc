#include "imalloc.h"
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct test
{
    u_int64_t number;
} test;

void monotonic_test()
{
    const size_t size = sizeof(imonotonic_heap) * 3;
    void* allocated_heap = (void*)malloc(size);
    imonotonic_heap* heap = imonotonic_heap_init(allocated_heap, size);
    printf("Size of test: %zu -- Size of size: %zu -- Heap size: %zu -- imonotonic_heap struct size: %zu\n", sizeof(test), size, heap->freesize, sizeof(imonotonic_heap));
    test* a = (test*)imonotonic_malloc(heap, sizeof(test));
    test* b = (test*)imonotonic_malloc(heap, sizeof(test));
    a->number = 12345678901234567890ULL;
    b->number = 12345678901234567890ULL;

    printf("Teste Number: %" PRIu64 "\n", a->number);
    printf("Heap size after alloc: %zu\n", heap->freesize);
    free(heap);
}

void buddy_test()
{
    const size_t size = 256;
    void* mem = malloc(size);
    ibuddy_heap* heap = ibuddy_heap_init(mem, size);
    /* printf("sz: %zu\n", heap->size); */
    void* result1 = ibuddy_malloc(heap, 14);
    if (result1 == NULL)
    {
        printf("NULL\n");
    }

    void* result2 = ibuddy_malloc(heap, 16);
    if (result2 == NULL)
    {
        printf("NULL\n");
    }
    printf("First block: %p - Second block: %p\n", result1, result2);
}

int main()
{
    buddy_test();
}
