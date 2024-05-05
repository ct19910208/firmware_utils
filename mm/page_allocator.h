#ifndef __PAGE_ALLOCATOR_H__
#define __PAGE_ALLOCATOR_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void* page_allocator_init(void* addr, size_t page_num, size_t page_size, size_t max_order);

void* alloc_pages(void* hnd, size_t size);

void* alloc_pages_order(void* hnd, size_t order);

void free_pages(void *hnd, void *addr);

void page_allocator_destroy(void* hnd);

size_t get_free_pages(void *hnd);

size_t get_free_size(void *hnd);

#ifdef CONFIG_PRINT_PAGE_ALLOCATOR
void print_page_allocator(void *hnd);
#endif

#ifdef __cplusplus
}
#endif
#endif
