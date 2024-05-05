#include "buddy.h"

#include "list.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BUDDY_MALLOC    malloc
#define BUDDY_FREE      free
#define BUDDY_ASSERT    assert
#define BUDDY_NULL      NULL
#define BUDDY_PRINTF    printf

enum page_flags_t {
    PG_HEAD,
    PG_TAIL,
    PG_BUDDY,
};

struct page_t {
    struct list_head lru;
    size_t flags;
    union {
        size_t order;
        struct page_t *first_page;
    };
};

struct free_area_t {
    struct list_head free_list;
    size_t nr_free;
};

struct mem_zone_t {
    size_t page_num;
    size_t page_size;
    size_t page_shift;
    struct page_t *page;
    size_t start_addr;
    size_t end_addr;
    size_t max_order;
    struct free_area_t *free_area;
};

static inline void _set_page_head(struct page_t *page)
{
    page->flags |= (1UL << PG_HEAD);
}

static inline void _set_page_tail(struct page_t *page)
{
    page->flags |= (1UL << PG_TAIL);
}

static inline void _set_page_buddy(struct page_t *page)
{
    page->flags |= (1UL << PG_BUDDY);
}

static inline void _clr_page_head(struct page_t *page)
{
    page->flags &= ~(1UL << PG_HEAD);
}

static inline void _clr_page_tail(struct page_t *page)
{
    page->flags &= ~(1UL << PG_TAIL);
}

static inline void _clr_page_buddy(struct page_t *page)
{
    page->flags &= ~(1UL << PG_BUDDY);
}

static inline size_t _get_page_head(struct page_t *page)
{
    return (page->flags & (1UL << PG_HEAD));
}

static inline size_t _get_page_tail(struct page_t *page)
{
    return (page->flags & (1UL << PG_TAIL));
}

static inline size_t _get_page_buddy(struct page_t *page)
{
    return (page->flags & (1UL << PG_BUDDY));
}

static inline void _set_page_order_buddy(struct page_t *page, size_t order)
{
    page->order = order;
    _set_page_buddy(page);
}

static inline void _rmv_page_order_buddy(struct page_t *page)
{
    page->order = 0;
    _clr_page_buddy(page);
}

static inline size_t _find_buddy_index(size_t page_idx, size_t order)
{
    return (page_idx ^ (1UL << order));
}

static inline size_t _find_combined_index(size_t page_idx, size_t order)
{
    return (page_idx & ~(1UL << order));
}

static inline size_t _get_combined_order(struct page_t *page)
{
    if (!_get_page_head(page))
        return 0;
    //return (size_t)page[1].lru.prev;
    return page->order;
}

static inline void _set_combined_order(struct page_t *page, size_t order)
{
    //page[1].lru.prev = (void *)order;
    page->order = order;
}

static inline size_t _page_is_combined(struct page_t *page)
{
    return (page->flags & ((1UL << PG_HEAD) | (1UL << PG_TAIL)));
}

static inline size_t _page_is_buddy(struct page_t *page, int order)
{
    return (_get_page_buddy(page) && (page->order == order));
}

static inline void *_page_to_addr(struct mem_zone_t *zone, struct page_t *page)
{
    size_t page_idx = page - zone->page;
    size_t address = zone->start_addr + page_idx * zone->page_size;
    return (void *)address;
}

static inline struct page_t *_addr_to_page(struct mem_zone_t *zone, void *addr)
{
    if (((size_t)addr < zone->start_addr) || ((size_t)addr > zone->end_addr)) {
        return BUDDY_NULL;
    }
    size_t page_idx = ((size_t)addr - zone->start_addr) >> zone->page_shift;
    struct page_t *page = &(zone->page[page_idx]);
    return page;
}

static void _prepare_combined_pages(struct page_t *page, size_t order)
{
    _set_combined_order(page, order);
    _set_page_head(page);
    /* not needed? */
    // for (int i = 1; i < (1 << order); i++) {
    //     struct page_t *p = page + i;
    //     _set_page_tail(&page[i]);
    //     page[i].first_page = page;
    // }
}

static void _expand(struct mem_zone_t *zone, struct free_area_t *area, struct page_t *page,
    size_t low_order, size_t high_order)
{
    size_t size = (1U << high_order);
    while (high_order > low_order) {
        area--;
        high_order--;
        size >>= 1;
        list_add(&page[size].lru, &area->free_list);
        area->nr_free++;
        // set page order
        _set_page_order_buddy(&page[size], high_order);
    }
}

static struct page_t *_alloc_pages(struct mem_zone_t *zone, size_t order)
{
    for (size_t i = order; i < zone->max_order; i++) {
        struct free_area_t *area = &(zone->free_area[i]);
        if (list_empty(&area->free_list)) {
            continue;
        }
        // remove closest size page
        struct page_t *page = list_entry(area->free_list.next, struct page_t, lru);
        list_del(&page->lru);
        _rmv_page_order_buddy(page);
        area->nr_free--;
        // expand to lower order
        _expand(zone, area, page, order, i);
        if (order > 0) {
            _prepare_combined_pages(page, order);
        } else {
            page->order = 0;
        }
        return page;
    }
    return BUDDY_NULL;
}

////////////////////////////////////////////////

static int _destroy_combined_pages(struct page_t *page, size_t order)
{
    int bad = 0;
    _clr_page_head(page);
    /* not needed? */
    // for (int i = 1; i < (1 << order); i++) {
    //     struct page_t *p = page + i;
    //     if (!_get_page_tail(&page[i]) || page[i].first_page != page) {
    //         bad++;
    //     }
    //     _clr_page_tail(p);
    // }
    return bad;
}

static void _buddy_free_pages(struct mem_zone_t *zone, struct page_t *page)
{
    size_t order = _get_combined_order(page);
    size_t buddy_idx = 0, combinded_idx = 0;
    size_t page_idx = page - zone->page;

    if (_page_is_combined(page))
        if (_destroy_combined_pages(page, order))
            ;

    while (order < zone->max_order - 1) {
        struct page_t *buddy;
        // find and delete buddy to combine
        buddy_idx = _find_buddy_index(page_idx, order);
        buddy = page + (buddy_idx - page_idx);
        if (!_page_is_buddy(buddy, order))
            break;
        list_del(&buddy->lru);
        zone->free_area[order].nr_free--;
        // remove buddy's flag and order
        _rmv_page_order_buddy(buddy);
        // update page and page_idx after combined
        combinded_idx = _find_combined_index(page_idx, order);
        page = page + (combinded_idx - page_idx);
        page_idx = combinded_idx;
        order++;
    }
    _set_page_order_buddy(page, order);
    list_add(&page->lru, &zone->free_area[order].free_list);
    zone->free_area[order].nr_free++;
}


void* page_allocator_init(void* addr, size_t page_num, size_t page_size, size_t max_order)
{
    /* initialize memory zone */
    struct mem_zone_t *zone = (struct mem_zone_t*)malloc(sizeof(struct mem_zone_t));
    BUDDY_ASSERT(BUDDY_NULL != zone);
    zone->page_num = page_num;
    zone->page_size = page_size;
    zone->page_shift = 0;
    size_t val = 1;
    for (int i = 0; i < 32; i++) {
        if (val >= page_size) {
            break;
        }
        val << 1;
        zone->page_shift++;
    }
    BUDDY_ASSERT(val == page_size);
    zone->start_addr = (size_t)addr;
    zone->end_addr = (size_t)addr + (size_t)page_num * (size_t)page_size;
    zone->max_order = max_order;
    /* initialize for free area */
    zone->free_area = (struct free_area_t*)malloc(max_order * sizeof(struct free_area_t));
    BUDDY_ASSERT(BUDDY_NULL != zone->free_area);
    for (int i = 0; i < max_order; i++) {
        INIT_LIST_HEAD(&(zone->free_area[i].free_list));
        zone->free_area[i].nr_free = 0;
    }
    /* initialize for pages */
    zone->page = (struct page_t*)malloc(page_num * sizeof(struct page_t));
    BUDDY_ASSERT(BUDDY_NULL != zone->page);
    memset(zone->page, 0, page_num * sizeof(struct page_t));
    for (int i = 0; i < page_num; i++) {
        INIT_LIST_HEAD(&(zone->page[i].lru));
        _buddy_free_pages(zone, &(zone->page[i]));
    }
    return (void*)zone;
}

void* alloc_pages(void* hnd, size_t size)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    size_t order = 0;
    size_t page_size = zone->page_size;
    while (page_size < size) {
        order++;
        page_size <<= 1;
    }
    if (order >= zone->max_order) {
        return BUDDY_NULL;
    }
    struct page_t* page = _alloc_pages(zone, order);
    return _page_to_addr(zone, page);
}

void* alloc_pages_order(void* hnd, size_t order)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    if (order >= zone->max_order) {
        return BUDDY_NULL;
    }
    struct page_t* page = _alloc_pages(zone, order);
    return _page_to_addr(zone, page);
}

void free_pages(void *hnd, void *addr)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    struct page_t *page = _addr_to_page(zone, addr);
    _buddy_free_pages(zone, page);
}

void page_allocator_destroy(void* hnd)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    BUDDY_FREE(zone->page);
    BUDDY_FREE(zone->free_area);
    BUDDY_FREE(zone);
}

size_t get_free_pages(void *hnd)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    size_t ret = 0;
    for (size_t i = 0; i < zone->max_order; i++) {
        ret += zone->free_area[i].nr_free * (1UL << i);
    }
    return ret;
}

size_t get_free_size(void *hnd)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    size_t pages = get_free_pages(hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    return pages * zone->page_size;
}

#ifdef CONFIG_PRINT_PAGE_ALLOCATOR
void print_page_allocator(void *hnd)
{
    BUDDY_ASSERT(BUDDY_NULL != hnd);
    struct mem_zone_t *zone = (struct mem_zone_t*)hnd;
    BUDDY_PRINTF("pages/free: %ld/%ld", zone->page_num, get_free_pages(hnd));
    BUDDY_PRINTF("order(pages) nr_free\n");
    for (size_t i = 0; i < BUDDY_MAX_ORDER; i++) {
        BUDDY_PRINTF("  %ld(%ld)  %ld\n", i, 1UL << i, zone->free_area[i].nr_free);
    }
}
#endif
