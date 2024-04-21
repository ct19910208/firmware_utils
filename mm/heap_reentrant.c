#include <stdint.h>
#include <stdlib.h>
#include "heap_reentrant.h"

#define HEAP_ASSERT
#define HEAP_ASSERT_MSG

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( hnd->xHeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;


typedef struct heap_reentrant {
    size_t xHeapStructSize;
	BlockLink_t xStart;
	BlockLink_t* pxEnd;
	size_t xFreeBytesRemaining;
	size_t xMinimumEverFreeBytesRemaining;
	size_t xBlockAllocatedBit;
} Heap_t;
typedef Heap_t* HeapHnd_t;

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void _insert_block_into_free_list(HeapHnd_t hnd, BlockLink_t *pxBlockToInsert );

/*
 * Called automatically to setup the required heap structures the first time
 * _malloc() is called.
 */
//static void _heap_init( void );

/*-----------------------------------------------------------*/

// /* The size of the structure placed at the beginning of each allocated memory
// block must by correctly byte aligned. */
// static const size_t xHeapStructSize	= ( sizeof( BlockLink_t ) + ( ( size_t ) ( HEAP_BYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) HEAP_BYTE_ALIGNMENT_MASK );

// /* Create a couple of list links to mark the start and end of the list. */
// static BlockLink_t xStart, *pxEnd = NULL;

// /* Keeps track of the number of free bytes remaining, but says nothing about
// fragmentation. */
// static size_t xFreeBytesRemaining = 0U;
// static size_t xMinimumEverFreeBytesRemaining = 0U;

// /* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
// member of an BlockLink_t structure is set then the block belongs to the
// application.  When the bit is free the block is still part of the free heap
// space. */
// static size_t xBlockAllocatedBit = 0;


/*-----------------------------------------------------------*/

void* malloc(void* heap,  size_t xWantedSize)
{
	HEAP_ASSERT(heap != NULL);
	HeapHnd_t hnd = (HeapHnd_t)heap;
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void *pvReturn = NULL;

//	vTaskSuspendAll();
	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( hnd->pxEnd == NULL )
		{
			HEAP_ASSERT_MSG(hnd->pxEnd != NULL, "heap is not initialized!");
			return NULL;
		}
		else
		{
//			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & hnd->xBlockAllocatedBit ) == 0 )
		{
			/* The wanted size is increased so it can contain a BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( xWantedSize > 0 )
			{
				xWantedSize += hnd->xHeapStructSize;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if( ( xWantedSize & HEAP_BYTE_ALIGNMENT_MASK ) != 0x00 )
				{
					/* Byte alignment required. */
					xWantedSize += ( HEAP_BYTE_ALIGNMENT - ( xWantedSize & HEAP_BYTE_ALIGNMENT_MASK ) );
					HEAP_ASSERT( ( xWantedSize & HEAP_BYTE_ALIGNMENT_MASK ) == 0 );
				}
				else
				{
//					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
//				mtCOVERAGE_TEST_MARKER();
			}

			if( ( xWantedSize > 0 ) && ( xWantedSize <= hnd->xFreeBytesRemaining ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &(hnd->xStart);
				pxBlock = hnd->xStart.pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != hnd->pxEnd )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + hnd->xHeapStructSize );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( BlockLink_t * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
						HEAP_ASSERT( ( ( ( size_t ) pxNewBlockLink ) & HEAP_BYTE_ALIGNMENT_MASK ) == 0 );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
						pxBlock->xBlockSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						_insert_block_into_free_list(hnd, pxNewBlockLink);
					}
					else
					{
//						mtCOVERAGE_TEST_MARKER();
					}

					hnd->xFreeBytesRemaining -= pxBlock->xBlockSize;

					if( hnd->xFreeBytesRemaining < hnd->xMinimumEverFreeBytesRemaining )
					{
						hnd->xMinimumEverFreeBytesRemaining = hnd->xFreeBytesRemaining;
					}
					else
					{
//						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xBlockSize |= hnd->xBlockAllocatedBit;
					pxBlock->pxNextFreeBlock = NULL;
				}
				else
				{
//					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
//				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
//			mtCOVERAGE_TEST_MARKER();
		}

//		traceMALLOC( pvReturn, xWantedSize );
	}
//	( void ) xTaskResumeAll();

//	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
//	{
//		if( pvReturn == NULL )
//		{
//			extern void vApplicationMallocFailedHook( void );
//			vApplicationMallocFailedHook();
//		}
//		else
//		{
//			mtCOVERAGE_TEST_MARKER();
//		}
//	}
//	#endif

	HEAP_ASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) HEAP_BYTE_ALIGNMENT_MASK ) == 0 );
	return pvReturn;
}
/*-----------------------------------------------------------*/

void free(void* heap, void *pv)
{
	HEAP_ASSERT(heap != NULL);
	HeapHnd_t hnd = (HeapHnd_t)heap;
    uint8_t *puc = ( uint8_t * ) pv;
    BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= hnd->xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( BlockLink_t * ) puc;

		/* Check the block is actually allocated. */
		HEAP_ASSERT( ( pxLink->xBlockSize & hnd->xBlockAllocatedBit ) != 0 );
		HEAP_ASSERT( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & hnd->xBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~(hnd->xBlockAllocatedBit);

//				vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					hnd->xFreeBytesRemaining += pxLink->xBlockSize;
//					traceFREE( pv, pxLink->xBlockSize );
					_insert_block_into_free_list(hnd, ( ( BlockLink_t * ) pxLink ) );
				}
//				( void ) xTaskResumeAll();
			}
			else
			{
//				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
//			mtCOVERAGE_TEST_MARKER();
		}
	}
}
/*-----------------------------------------------------------*/

size_t get_free_heap_size( void* heap )
{
	HEAP_ASSERT(heap != NULL);
	HeapHnd_t hnd = (HeapHnd_t)heap;
	return hnd->xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

size_t get_min_ever_free_heap_size( void* heap )
{
	HEAP_ASSERT(heap != NULL);
	HeapHnd_t hnd = (HeapHnd_t)heap;
	return hnd->xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

void _initialize_blocks( HeapHnd_t hnd )
{
	/* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

void* heap_init(void* addr, uint32_t size)
{
	HEAP_ASSERT( size > (sizeof(Heap_t) + 8 + HEAP_BYTE_ALIGNMENT + 2 * sizeof(BlockLink_t) ) );
	/* memory for Heap_t */
	size_t hnd_addr = (size_t)addr;
	if( ( hnd_addr & 0x7 ) != 0 )
	{
		hnd_addr += ( 8 - 1 );
		hnd_addr &= ~( ( size_t ) 0x7 );
	}

	HeapHnd_t hnd = (HeapHnd_t)hnd_addr;
	hnd->xHeapStructSize = ( sizeof( BlockLink_t ) + ( ( size_t ) ( HEAP_BYTE_ALIGNMENT - 1 ) ) ) & ~( ( size_t ) HEAP_BYTE_ALIGNMENT_MASK );
	hnd->xFreeBytesRemaining = 0;
	hnd->xMinimumEverFreeBytesRemaining = 0;
	hnd->xBlockAllocatedBit = 0;

	size_t base_addr = hnd_addr + sizeof(Heap_t);

    BlockLink_t *pxFirstFreeBlock;
    uint8_t *pucAlignedHeap;
    size_t uxAddress;
    size_t xTotalHeapSize = size - (base_addr - (size_t)addr);

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = base_addr;

	if( ( uxAddress & HEAP_BYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( HEAP_BYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) HEAP_BYTE_ALIGNMENT_MASK );
		xTotalHeapSize -= uxAddress - base_addr;
	}

	pucAlignedHeap = ( uint8_t * ) uxAddress;

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	hnd->xStart.pxNextFreeBlock = ( BlockLink_t * ) pucAlignedHeap;
	hnd->xStart.xBlockSize = ( size_t ) 0;

	/* pxEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	uxAddress = ( ( size_t ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= hnd->xHeapStructSize;
	uxAddress &= ~( ( size_t ) HEAP_BYTE_ALIGNMENT_MASK );
	hnd->pxEnd = ( BlockLink_t * ) uxAddress;
	hnd->pxEnd->xBlockSize = 0;
	hnd->pxEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxEnd. */
	pxFirstFreeBlock = ( BlockLink_t * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( size_t ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = hnd->pxEnd;

	/* Only one block exists - and it covers the entire usable heap space. */
	hnd->xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	hnd->xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a size_t variable. */
	hnd->xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );

	return (void*)hnd;
}
/*-----------------------------------------------------------*/

static void _insert_block_into_free_list(HeapHnd_t hnd, BlockLink_t *pxBlockToInsert )
{
    BlockLink_t *pxIterator;
    uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &(hnd->xStart); pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
	else
	{
//		mtCOVERAGE_TEST_MARKER();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != hnd->pxEnd )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = hnd->pxEnd;
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
	else
	{
//		mtCOVERAGE_TEST_MARKER();
	}
}

