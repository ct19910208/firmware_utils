#ifndef __MALLOC_H__
#define __MALLOC_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void HeapInit(void);

/* will be deprecated */
void HeapSet(void* addr, uint32_t size);

void* Malloc(size_t size);

void Free(void* addr);

uint32_t HeapGetFreeSize(void);

uint32_t HeapGetMinEverFreeSize(void);

#ifdef __cplusplus
}
#endif
#endif
