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

int main()
{
    const size_t size = sizeof(imonotonic_heap) * 3;
    void* allocated_heap = (void*)malloc(size);
    imonotonic_heap* heap = imonotonic_heap_init(allocated_heap, size);
    printf("Size of test: %zu -- Size of size: %zu -- Heap size: %zu\n", sizeof(test), size, heap->freesize);
    test* a = (test*)imonotonic_malloc(heap, sizeof(test));
    test* b = (test*)imonotonic_malloc(heap, sizeof(test));
    a->number = 12345678901234567890ULL;
    b->number = 12345678901234567890ULL;

    printf("Teste Number: %" PRIu64 "\n", a->number);
    printf("Heap size after alloc: %zu\n", heap->freesize);
    free(heap);
}
