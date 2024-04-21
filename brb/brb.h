
#ifndef __BRB_H__
#define __BRB_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct brb_t {
	void                *buf;
	volatile uint32_t   p_rd;
	volatile uint32_t   p_wr;
	volatile uint32_t   blk_cnt;
	volatile uint32_t   blk_size;
	volatile uint32_t   rd_pos;
	volatile uint32_t   wr_pos;
};

typedef struct brb_t* brb_hnd_t;

void brb_init(brb_hnd_t brb, uint32_t blk_cnt, uint32_t blk_size, void *buf);

void brb_destroy(brb_hnd_t brb);

void* brb_get_rd_ptr(brb_hnd_t brb);

void brb_pop_rd_ptr(brb_hnd_t brb);

void* NpuBrbGetWrPtr(brb_hnd_t brb);

void brb_pop_wr_ptr(brb_hnd_t brb);

#ifdef __cplusplus
}
#endif
#endif
