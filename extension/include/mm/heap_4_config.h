
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

void *mem_4_malloc(size_t xWantedSize);
void mem_4_free(void *pv);
size_t mem_4_free_get(void);

#endif


