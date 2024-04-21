#include "brb.h"

#define BRB_ASSERT assert

void brb_init(brb_hnd_t brb, uint32_t blk_cnt, uint32_t blk_size, void *buf)
{
    BRB_ASSERT(0 != blk_cnt);
    BRB_ASSERT(NULL != buf);
    BRB_ASSERT(NULL != brb);
    brb->buf = buf;
    brb->p_rd = (uint32_t)brb->buf;
    brb->p_wr = (uint32_t)brb->buf;
    brb->blk_size = blk_size;
    brb->blk_cnt = blk_cnt;
    brb->rd_pos = 0;
    brb->wr_pos = 0;
}

void brb_destroy(brb_hnd_t brb)
{

}

void* brb_get_rd_ptr(brb_hnd_t brb)
{
    BRB_ASSERT(NULL != brb);
    BRB_ASSERT(NULL != brb->buf);

    if (brb->wr_pos - brb->rd_pos > 0)
        return (void*)brb->p_rd;

    /* Means that the ring buffer is empty. */
    return NULL;
}

void brb_pop_rd_ptr(brb_hnd_t brb)
{
    BRB_ASSERT(NULL != brb);
    BRB_ASSERT(NULL != brb->buf);
    /* Check validity before read */
    BRB_ASSERT(brb->wr_pos - brb->rd_pos > 0);

    brb->p_rd += brb->blk_size;
    /* Assumed that blk_size*blk_cnt will not overflow! And it should be! */
    if (brb->p_rd >= ((uint32_t)brb->buf) + (brb->blk_size * brb->blk_cnt))
        brb->p_rd = (uint32_t)brb->buf;
    brb->rd_pos++;
}

void* brb_get_wr_ptr(brb_hnd_t brb)
{
    BRB_ASSERT(NULL != brb);
    BRB_ASSERT(NULL != brb->buf);

    if (brb->wr_pos - brb->rd_pos < brb->blk_cnt)
        return (void*)brb->p_wr;

    /* Means that the buffer is full or overflowed. */
    return NULL;
}

void brb_pop_wr_ptr(brb_hnd_t brb)
{
    BRB_ASSERT(NULL != brb);
    BRB_ASSERT(NULL != brb->buf);
    /* Check validity before write */
    BRB_ASSERT(brb->wr_pos - brb->rd_pos < brb->blk_cnt);
        
    brb->p_wr += brb->blk_size;
    if (brb->p_wr >= ((uint32_t)brb->buf) + ( brb->blk_size * brb->blk_cnt ))
        brb->p_wr = (uint32_t)brb->buf;
    ++brb->wr_pos;
}
