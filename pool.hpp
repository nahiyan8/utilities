#ifndef POOL_HPP_INCLUDED
#define POOL_HPP_INCLUDED

///
/// Summary:
///     1. A "pool" of elements is preallocated.
///     2. A user allocates one of the slots, note that allocating just means
///        marking the space as used.
///     3. The user uses it, and the allocator makes sure that their space isn't
///        given out to someone else.
///     4. It's deallocated (marked as free), which frees it for other users.
///

///
/// Variables:
///     * Pool elements are stored in the dynamically-allocated array (T*) pool.
///     * Usage marks are stored as bits in the (uint32_t*) used array.
///     * (size_t) pool_size stores the size for pool, and used_size for used.
///     * (size_t) used_count keeps track of the number of elements allocated.
///     * (size_t) allocator_tab keeps the index of the last-deallocated element,
///       or the index which the allocator searched upto until it found space.
///     * (bool) initialised remembers if create() was called for init.
///

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// ---- DECLARATIONS ---- ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "diagnostics.hpp"

#define FREE 0
#define USED 1

#define POOL_ERROR ~((size_t) 0UL)

// http://www-graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
// For Log2(x), the bit's position. It uses about 13 operations, and it's used
// in here to find a 1 or 0 bit in *used, in findFree() and findUsed().
static const uint MultiplyDeBruijnBitPosition2[32] = 
{
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

template <class T> class Pool
{
	public:
		/* -- Variables -- */
		T        *pool; size_t pool_size;
		uint32_t *used; size_t used_size;
		
		size_t used_count;  // A count of the number of slots allocated.
		uint32_t* free_ptr; // used_ptr to the lowest-known free-space.
		bool initialised;
		
		/* -- Public functions -- */
		 Pool();
		 Pool(size_t _size);
		~Pool();
		
		void create (size_t _size);
		void destroy();
		
		size_t allocate(size_t count = 1);
		void deallocate(size_t slot);

		size_t findUsed(uint32_t **start_ptr = NULL) const;
		
		inline bool isFree(size_t slot) const;
		inline bool isUsed(size_t slot) const;
		
		inline size_t size () const { return pool_size;  };
		inline size_t count() const { return used_count; };
		
		inline T& operator[](uint slot);
		
		void iterate(void (*func_ptr)(Pool<T> *ptr, size_t slot));
};

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// ---- IMPLEMENTATION ---- ///////////////////////////
///////////////////// --- CONSTRUCTOR/S & DESTRUCTOR/S --- /////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class T> Pool<T>::Pool() : initialised(false) {}

////////////////////////////////////////////////////////////////////////////////

template <class T> Pool<T>::Pool(size_t _size) : initialised(false)
{
	create(_size);
}

////////////////////////////////////////////////////////////////////////////////

template <class T> Pool<T>::~Pool()
{
	destroy();
}

////////////////////////////////////////////////////////////////////////////////

template <class T> void Pool<T>::create(size_t _pool_size)
{
	if (initialised)
		return;
	
	pool_size  = _pool_size;
	used_size  = (_pool_size >> 5) + ((_pool_size & 31) != 0);
	used_count = 0;
	
	pool = NULL;
	used = NULL;
	
	try
	{
		pool = new T        [pool_size];
		used = new uint32_t [used_size];
		
		memset(used, 0x00000000, used_size);
	}
	catch ( ... )
	{
		ReportLog(GAME_LOG_FATAL, "exception thrown on pool allocation");
		destroy();
	}
	
	initialised = true;
}

////////////////////////////////////////////////////////////////////////////////

template <class T> void Pool<T>::destroy()
{
	if (pool) delete [] pool;
	if (used) delete [] used;
	
	pool_size  = 0;
	used_size  = 0;
	used_count = 0;
	
	initialised = false;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// --- (DE)ALLOCATION --- ////////////////////////////
///////////////////////// -- MEMORY (DE)ALLOCATOR/S -- /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class T> size_t Pool<T>::allocate(size_t count)
{
	uint32_t *used_ptr = free_ptr, *used_end = (used + used_size);
	uint32_t temp;
	
	if (count == 0)
		return POOL_ERROR;
	
	for (; used_ptr != used_end; used_ptr++) // Loop through the array.
	{
		AllocateMore:
		
		if (*used_ptr != 0xffffffffU)
		{
			temp       = *used_ptr;
			
			// 01100111 + -> 01101000; Adding one sets the lowest 0.
			// 01100111 | -> 01101111; ORing gets back the trailing 1s.
			// 01101111 ^ -> 00001000; XORing gets a mask of the changed bit.
			*used_ptr |= *used_ptr + 1U;
			temp      ^= *used_ptr;
			
			used_count++;
			     count--;
			
			if (count)
				goto AllocateMore;
			
			free_ptr = used_ptr;
			
			// Remember, when finding out the index by getting the difference,
			// account for the fact that they're 4-bytes long: adjust the shift.
			return   ((size_t)(used_ptr - used) << 3)
			       + (MultiplyDeBruijnBitPosition2[(temp * 0x077CB531U) >> 27]);
		}
	}
	
	// It didn't return from inside the loop, so it didn't find a 0 bit.
	return POOL_ERROR;
}

////////////////////////////////////////////////////////////////////////////////

template <class T> void Pool<T>::deallocate(size_t slot)
{
	if (!initialised || slot >= pool_size)
		return;

	
	uint32_t *used_ptr = used + (slot >> 5);
	
	*used_ptr &= ~(1 << (slot & 31));
	used_count--;
	
	// Remember that this has become a free slot, and always keep the lowest ptr
	if (used_ptr < free_ptr)
		free_ptr = used_ptr;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////// -- ALLOCATION INFORMATION -- /////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class T> size_t Pool<T>::findUsed(uint32_t **start_ptr) const
{
	uint32_t *used_ptr, *used_end = (used + used_size);
	uint32_t temp;
	
	// Continue from *start_ptr if start_ptr != NULL, otherwise start from used.
	used_ptr = start_ptr ? *start_ptr : used;
	
	while (used_ptr++ != used_end)
		if (*used_ptr) // != 0x00000000
		{
			if (start_ptr)
				*start_ptr = used_ptr;
			
			temp = *used_ptr;
			temp ^= temp & (temp - 1);
			
			// + MultiplyDeBruijnBitPosition2[...] is the subindex.
			return ((size_t)(used_ptr - used) << 3) + MultiplyDeBruijnBitPosition2[(temp * 0x077CB531U) >> 27];
		}
	
	return POOL_ERROR;
}

////////////////////////////////////////////////////////////////////////////////

template <class T> bool Pool<T>::isFree(size_t slot) const
{
	if (slot >= pool_size)
		return false;
	
	// IF YOU UNDERSTAND THIS, THEN YOU ARE A BLACK BELT OFFICIAL BINARY NINJA!
	return (bool) (~(used[slot >> 5] >> (slot & 31)) & 1);
}

////////////////////////////////////////////////////////////////////////////////

template <class T> bool Pool<T>::isUsed(size_t slot) const
{
	if (slot >= pool_size)
		return false;
	
	// Notice the missing ~, the above function called for the opposite of "used", therefore the NOT.
	// [slot >> 5] == (slot / 32); slot is in bits, so this is to convert it to an index. (int = 32b)
	// (slot & 31) == (slot % 32); It's position of the bit we're trying to retrieve.
	// The shift (>>) shifts the word by the bit's position, so the bit is now the least-significant.
	// The AND (& 1) zeroes out all bits except for the least-significant one. Now, typecast to bool.
	// You're welcome, grasshopper. Now gimme 10 bucks. Don't call us at 1-800-IAM-SCAM to complain.
	return (bool) ((used[slot >> 5] >> (slot & 31)) & 1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// --- OPERATOR/S --- //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class T> inline T& Pool<T>::operator[](uint slot)
{
	return pool[slot];
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////// --- UTILITY FUNCTION/S --- //////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <class T> void Pool<T>::iterate(void (*func_ptr)(Pool<T> *ptr, size_t slot))
{
	uint32_t *used_ptr = used, *used_end = (used + used_size);
	uint32_t index_in_bits, temp, temp2;
	
	while (used_ptr++ != used_end)
		if (*used_ptr)
		{
			temp = *used_ptr;
			index_in_bits = (size_t)(used_ptr - used) << 3;
			
			while (temp)
			{                      // Eg:  |
				temp2  = temp;     // 10110100
				temp  &= temp - 1; // 10110000 -- least-significant 1 cleared.
				temp2 ^= temp;     // 00000100 -- a mask for the bit
				
				// temp2 now contains the subindex.
				temp2  = MultiplyDeBruijnBitPosition2[(temp2 * 0x077CB531U) >> 27];
				temp2 += index_in_bits;
				
				func_ptr(this, temp2);
			}
		}
}

////////////////////////////////////////////////////////////////////////////////

#endif // POOL_HPP_INCLUDED
