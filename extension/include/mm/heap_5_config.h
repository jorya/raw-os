
#ifndef HEAP_4_CONFIG_H
#define HEAP_4_CONFIG_H


#define portBYTE_ALIGNMENT          4

#define configTOTAL_HEAP_SIZE       4096

#if (portBYTE_ALIGNMENT == 8)
	#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if (portBYTE_ALIGNMENT == 4)
	#define portBYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if (portBYTE_ALIGNMENT == 2)
	#define portBYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if (portBYTE_ALIGNMENT == 1)
	#define portBYTE_ALIGNMENT_MASK	( 0x0000 )
#endif

#ifndef portPOINTER_SIZE_TYPE
	#define portPOINTER_SIZE_TYPE unsigned long
#endif


/* Used by heap_5.c. */
typedef struct HeapRegion
{
	RAW_U8 *pucStartAddress;
	size_t xSizeInBytes;
} HeapRegion_t;


void *mem_5_malloc(size_t xWantedSize);
void mem_5_free(void *pv);
size_t mem_5_free_get(void);

void mem_5_heap_regions(const HeapRegion_t * const pxHeapRegions);
size_t mem_5_ever_free_heap(void);

#endif


