/**************************************************************************
	* Change the code and modify the heap_4 file from freeRtos. 
	* On this basis, I modified it to an object-oriented and management method, 
	* although it is not thread-safe now!
*/

/**************************************************************************
  * @brief : task system Memory management
  * @author : Waies
  * @copyright :  
  * @version :v1.0
  * @note : 
  * @history : 
*/

/*-----------------------------------------------------------
 * Portable layer API.  Each function must be defined for each port.
 *----------------------------------------------------------*/

#ifndef PORTABLE_H
#define PORTABLE_H

/* Each FreeRTOS port has a unique portmacro.h header file.  Originally a
pre-processor definition was used to ensure the pre-processor found the correct
portmacro.h file for the port being used.  That scheme was deprecated in favour
of setting the compiler's include path such that it found the correct
portmacro.h file - removing the need for the constant and allowing the
portmacro.h file to be located anywhere in relation to the port being used.
Purely for reasons of backward compatibility the old method is still valid, but
to make it clear that new projects should not use it, support for the port
specific constants has been moved into the deprecated_definitions.h header
file. */


/* If portENTER_CRITICAL is not defined then including deprecated_definitions.h
did not result in a portmacro.h header file being included - and it should be
included here.  In this case the path to the correct portmacro.h header file
must be set in the compiler's include path. */

#define portBYTE_ALIGNMENT						    8
#define configASSERT( x ) if ((x) == 0)			   { for( ;; );} 
#define mtCOVERAGE_TEST_MARKER()

#if portBYTE_ALIGNMENT == 32
	#define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif

#if portBYTE_ALIGNMENT == 16
	#define portBYTE_ALIGNMENT_MASK ( 0x000f )
#endif

#if portBYTE_ALIGNMENT == 8
	#define portBYTE_ALIGNMENT_MASK ( 0x0007 )
#endif

#if portBYTE_ALIGNMENT == 4
	#define portBYTE_ALIGNMENT_MASK	( 0x0003 )
#endif

#if portBYTE_ALIGNMENT == 2
	#define portBYTE_ALIGNMENT_MASK	( 0x0001 )
#endif

#if portBYTE_ALIGNMENT == 1
	#define portBYTE_ALIGNMENT_MASK	( 0x0000 )
#endif


#ifndef portNUM_CONFIGURABLE_REGIONS
	#define portNUM_CONFIGURABLE_REGIONS 1
#endif

#ifndef portHAS_STACK_OVERFLOW_CHECKING
	#define portHAS_STACK_OVERFLOW_CHECKING 0
#endif

#ifndef portARCH_NAME
	#define portARCH_NAME NULL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK* pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;


typedef struct memSupervise_Bind
{
	/* Must be a global array */
	uint8_t* ucHeap;
	size_t sizeHeap;
	/* Create a couple of list links to mark the start and end of the list. */
	BlockLink_t xStart;
	BlockLink_t* pxEnd;
	/* Keeps track of the number of free bytes remaining,
	but says nothing aboutfragmentation. */
	size_t xFreeBytesRemaining;
	size_t xMinimumEverFreeBytesRemaining;
	/*When the bit is free the block is still part of the free heap space.*/
	size_t xBlockAllocatedBit;

}hpSeBind;

/*
 * Map to the memory management routines required for the port.
 */

void initHp(hpSeBind* hpBase, uint8_t* heapArr, size_t hpsize);
void *hpNew(hpSeBind* hpBase, size_t xSize )  ;
void hpdel(hpSeBind* hpBase, void *pv )  ;

size_t getFreehp(hpSeBind* hpBase)  ;
size_t getMinihp(hpSeBind* hpBase)  ;


/*
 * The structures and methods of manipulating the MPU are contained within the
 * port layer.
 *
 * Fills the xMPUSettings structure with the memory region information
 * contained in xRegions.
 */
#if( portUSING_MPU_WRAPPERS == 1 )
	struct xMEMORY_REGION;
	void vPortStoreTaskMPUSettings( xMPU_SETTINGS *xMPUSettings, const struct xMEMORY_REGION * const xRegions, StackType_t *pxBottomOfStack, uint32_t ulStackDepth ) PRIVILEGED_FUNCTION;
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTABLE_H */

