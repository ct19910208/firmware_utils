#ifndef __HEAP_REENTRANT_H__
#define __HEAP_REENTRANT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

/* Byte alignment definition */
#define HEAP_BYTE_ALIGNMENT  (32)

#if HEAP_BYTE_ALIGNMENT == 32
    #define HEAP_BYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if HEAP_BYTE_ALIGNMENT == 16
    #define HEAP_BYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if HEAP_BYTE_ALIGNMENT == 8
    #define HEAP_BYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if HEAP_BYTE_ALIGNMENT == 4
    #define HEAP_BYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if HEAP_BYTE_ALIGNMENT == 2
    #define HEAP_BYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if HEAP_BYTE_ALIGNMENT == 1
    #define HEAP_BYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef HEAP_BYTE_ALIGNMENT_MASK
    #error "Invalid HEAP_BYTE_ALIGNMENT definition"
#endif




void* heap_init(void* addr, uint32_t size);

void* malloc(void* heap, size_t xWantedSize);

void free(void* heap, void *pv);

size_t get_free_heap_size(void* heap);

size_t get_min_ever_free_heap_size(void* heap);

#ifdef __cplusplus
}
#endif
#endif
