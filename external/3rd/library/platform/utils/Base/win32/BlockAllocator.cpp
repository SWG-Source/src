#include "../BlockAllocator.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

	BlockAllocator::BlockAllocator()
	{
		for(unsigned i = 0; i < 31; i++)
		{
			m_blocks[i] = NULL;
		}
	}

	BlockAllocator::~BlockAllocator()
	{
		// free all allocated memory blocks
		for(unsigned i = 0; i < 31; i++)
		{
			while(m_blocks[i] != NULL)
			{
				uintptr_t *tmp = m_blocks[i];
				m_blocks[i] = (uintptr_t *)*m_blocks[i];
				free(tmp);
			}
		}
	}

// Allocate a block that is the next power of two greater than the # of bytes passed.
// 33 bytes yields a 64 byte block of memory and so forth.

	void *BlockAllocator::getBlock(unsigned bytes)
	{
		unsigned accum = 16, bits = 16;
		uintptr_t *handle = NULL;

		// Perform a binary search looking for the highest bit.

		while(bits != 0)
		{
			// If bytes is less than the bit we're testing for, subtract half
			// from the bit value and repeat
			if(bytes < (unsigned)(1 << accum))
			{
				bits /= 2;
				accum -= bits;
			}
			// If bytes is greater than the bit we're testing for, add half
			// from the but value and repeat
			else if(bytes > (unsigned)(1 << accum))
			{
				bits /= 2;
				accum += bits;
			}
			// Got lucky and hit the value dead on
			else
			{
				break;
			}
		}
		// At this point accum contains the most significant bit index, increment
		accum++;
		if(accum < 2)
		{
			accum = 2;
		}
	
		// Note that when memory is actually allocated, 8 extra bytes will be allocated.at the front
		// The first integer is the address of the next block of memory when the block is in the allocator
		// The second integer is the bit length of the block
		// Memory is allocated on 4 byte boundaries to sidestep byte alignment problems


		// Check if the allocator already has a block of that size
		if(m_blocks[accum] == 0)
		{
			// remove the pre allocated block from the linked list
			handle = (uintptr_t *)calloc(((1 << accum) / 4) + 2, sizeof(unsigned));
			handle[1] = accum;
			handle[0] = 0;
		}
		else
		{
			// Allocate a new block 
			handle = m_blocks[accum];
			m_blocks[accum] = (uintptr_t *)handle[0];
			handle[0] = 0;
		}
		// return a pointer that skips over the header used for the allocator's purposes
		return(handle + 2);
	}

	void BlockAllocator::returnBlock(unsigned *handle)
	{
		// C++ allows for safe deletion of a NULL pointer
		if(handle)
		{
			// Update the allocator linked list, insert this entry at the head
			*(handle - 2) = (uintptr_t)m_blocks[*(handle - 1)];
			// Add this entry to the proper linked list node
			m_blocks[*(handle - 1)] = (handle - 2);
		}
	}
};
#ifdef EXTERNAL_DISTRO
};
#endif

