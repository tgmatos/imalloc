#include "imalloc.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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

void test_allocator_basic_allocation()
{
    size_t heap_size = 256;
    void* mem = malloc(heap_size);
    ibuddy_heap* heap = ibuddy_heap_init(mem, heap_size);

    void* ptr = ibuddy_malloc(heap, 32);
    assert(ptr != NULL && "Allocation failed for 32 bytes");

    ibuddy_block* block = (ibuddy_block*)((char*)ptr - sizeof(ibuddy_block));
    assert(block->used == true);
    assert(block->size >= 32); // Buddy allocator may allocate a power-of-two block

    free(mem);
    printf("test_allocator_basic_allocation passed.\n");
}

void test_allocator_multiple_allocations()
{
    size_t heap_size = 256;
    void* mem = malloc(heap_size);
    ibuddy_heap* heap = ibuddy_heap_init(mem, heap_size);

    void* p1 = ibuddy_malloc(heap, 32);
    void* p2 = ibuddy_malloc(heap, 32);
    void* p3 = ibuddy_malloc(heap, 32);

    assert(p1 != NULL && p2 != NULL && p3 != NULL);

    ibuddy_block* b1 = (ibuddy_block*)((char*)p1 - sizeof(ibuddy_block));
    ibuddy_block* b2 = (ibuddy_block*)((char*)p2 - sizeof(ibuddy_block));
    ibuddy_block* b3 = (ibuddy_block*)((char*)p3 - sizeof(ibuddy_block));

    assert(b1->used && b2->used && b3->used);
    assert(b1 != b2 && b2 != b3 && b1 != b3); // Ensure different blocks

    ibuddy_free_sized(heap, p1, 32);
    ibuddy_free_sized(heap, p2, 32);
    ibuddy_free_sized(heap, p3, 32);

    free(mem);
    printf("test_allocator_multiple_allocations passed.\n");
}

void test_allocator_out_of_memory()
{
    size_t heap_size = 128;
    void* mem = malloc(heap_size);
    ibuddy_heap* heap = ibuddy_heap_init(mem, heap_size);

    void* p1 = ibuddy_malloc(heap, 64);
    void* p2 = ibuddy_malloc(heap, 64);
    void* p3 = ibuddy_malloc(heap, 64); // Likely to fail due to insufficient memory

    assert(p1 != NULL && p2 != NULL);
    assert(p3 == NULL && "Should fail allocation due to out of memory");

    free(mem);
    printf("test_allocator_out_of_memory passed.\n");
}

int main()
{
    test_allocator_basic_allocation();
    test_allocator_multiple_allocations();
    test_allocator_out_of_memory();
}
