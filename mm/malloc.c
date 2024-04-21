#include "malloc.h"

#define MALLOC_ASSERT

#ifndef LOCAL_HEAP_SIZE
#define LOCAL_HEAP_SIZE (16 * 1024)
#endif

void* heap_hnd = NULL;

static void* heap_addr = NULL;
static uint32_t heap_size = 0;
static uint8_t local_heap[LOCAL_HEAP_SIZE];

void HeapInit(void)
{
    heap_addr = local_heap;
    heap_size = LOCAL_HEAP_SIZE;
    heap_hnd = heap_init(heap_addr, heap_size);
}

/* will be deprecated */
void HeapSet(void* addr, uint32_t size)
{
    heap_addr = addr;
    heap_size = size;
    if ((heap_addr != NULL) && (heap_size != 0)) {
        LOG_INFO("[HEAP]", "Set heap, addr=0x%x, size=%d\n", addr, size);
        heap_hnd = heap_init(heap_addr, heap_size);
    } else {
        LOG_ERROR("[HEAP]", "Invalid heap address and/or size\n");
    }
}

void* Malloc(size_t size)
{
    MALLOC_ASSERT(size > 0);
    MALLOC_ASSERT((heap_addr != NULL) && (heap_size != 0));
    void* addr = malloc(heap_hnd, size);
    MALLOC_ASSERT(addr != NULL);
    return addr;
}

void Free(void* addr)
{
    MALLOC_ASSERT(addr != NULL);
    free(heap_hnd, addr);
}

uint32_t HeapGetFreeSize(void) 
{
    return get_free_heap_size(heap_hnd);
}

uint32_t HeapGetMinEverFreeSize(void) 
{
    return get_min_ever_free_heap_size(heap_hnd);
}
