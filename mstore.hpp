#ifndef UTILS_MSTORE_HPP_INCLUDED
#define UTILS_MSTORE_HPP_INCLUDED

// C++ version of stdlib.h used, so that 
#include <cstdlib>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

//
// NOTE: A 1-bit in *free means that slot is free to be allocated.
// FIXME: The class has not been tested, and iterate() specifically is malfunctioning. Needs testing.
//

// Some constants. (MS stands for mstore)
#define MS_SHIFT 6
#define MS_MASK  0x3F // 0b00111111 (six set bits)
typedef uint64_t msfree_t;

template <class T>
class mstore
{
	private:
		T        *data;
		msfree_t *free;
		msfree_t *last; // last keeps a pointer to the last known free slot bit in msfree_t *free.
		
		size_t dsize, fsize; // d[ata]-size and f[ree]-size, repectively.
		
	public:
		 mstore()                { dsize = 0; fsize = 0; free = NULL; data = NULL; last = NULL;                   }
		 mstore(size_t new_size) { dsize = 0; fsize = 0; free = NULL; data = NULL; last = NULL; resize(new_size); }
		~mstore()                { release();                                                                     }
		
		bool resize(size_t new_size);
		void release();
		
		size_t allocate();
		void   deallocate(size_t slot);
		
		void iterate (void (*function) (T *object_ptr, void *argument), void *argument);
		
		inline T& operator[] (size_t slot) { return data[slot]; };
};

///////////////////////////////////////////////////////////////////////////////
////////////////////////// ---- IMPLEMENTATIONS ---- //////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <class T>
bool mstore<T>::resize(size_t new_dsize)
{
	// The size of the new free array is (one-bit per object / bits per msfree_t) + (1 extra msfree_t for leftover bits)
	size_t new_fsize = ((new_dsize >> MS_SHIFT) + ((new_dsize & MS_MASK) != 0)) * sizeof(msfree_t);
	
	free = (msfree_t*) std::realloc(free, new_fsize            );
	data = (T*)        std::realloc(data, new_dsize * sizeof(T));
	
	// If the new_dsize wasn't deliberately 0, then check if any of the reallocs returned NULLs.
	if (new_dsize != 0 && (free == NULL || data == NULL))
	{
		// TODO: add logging here: Memory allocation failed.
		release();
		
		return false;
	}
	
	// Set any newly allocated bytes to their defaults: set all new slots to free, and data to 0.
	if (new_dsize > dsize)
	{
		memset(free + fsize, 0xFF, (new_fsize - fsize)            );
		memset(data + dsize, 0x00, (new_dsize - dsize) * sizeof(T));
	}
	
	dsize = new_dsize;
	fsize = new_fsize;
	last  = free;
	
	return true;
}

// ------------------------------------------------------------------------- //

template <class T>
void mstore<T>::release()
{
	std::free(free);
	std::free(data);
	
	free = NULL;
	data = NULL;
	last = NULL;
	
	dsize = 0;
	fsize = 0;
}

// ------------------------------------------------------------------------- //

// Example of what mstore::allocate()'s assembly code should look like after
// the variable declarations/initialisations. (except for last = ptr)

// NOTE: GCC isn't using the LOOP instruction, instead using another
//       register to increment upto count, testing for equality each loop.

/*
__asm__ __volatile__ (
                      "jrcxz	2f"                  "\n\t" // Use either EDX or RDX.
                      "1$:"                          "\n\t"
                        "mov	edx, DWORD PTR [%0]" "\n\t" // DWORD or QWORD.
                        "test	edx, edx"            "\n\t"
                        "jnz	3f"                  "\n\t"
                        "add	%0, %4"              "\n\t"
                        "loop	1b"                  "\n\t"
                      "2$:"                          "\n\t"
                        "xor	%2, %2"              "\n\t"
                        "jmp 4f"                     "\n\t"
                      "3$:"                          "\n\t"
                        "lea	%1, [edx - 1]"       "\n\t"
                        "bsf	%2, edx"             "\n\t"
                        "and	edx, %1"             "\n\t"
                        "mov	DWORD PTR [%0], edx" "\n\t" // DWORD or QWORD.
                        "sub	%0, %3"              "\n\t"
                        "shr	%0, %5"              "\n\t" // Equivalent to log2(sizeof(msfree_t)).
                        "add	%2, %0"              "\n\t"
                      "4$:"                          "\n\t"
                        "ret"
                      : "+r" (ptr), "+c" (count), "=a" (slot)
                      : "rm" (free), "i" (sizeof(msfree_t)), "I" (MS_SHIFT) // "I" = 32-bit, "J" = 64-bit shifts.
                      : "edx", "memory", "cc"
                     );
*/

template <class T>
size_t mstore<T>::allocate()
{
	// Start checking from the last pointer, and note: ptr1 - ptr2 = number of elements, not bytes.
	msfree_t *ptr   = last;
	size_t    count = fsize - (free - last);
	
	msfree_t temp;
	size_t   slot;
	
	// Loop across the array and look for a free (1) bit. Then *ptr should be non-zero.
	while (count--)
	{
		if (*ptr)
			goto found;
		
		ptr++;
	}
	
	// TODO: add logging here: Slot allocation/search failed.
	{
		// Retry might possibly yield free slots, if the last pointer caused us to skip
		// some free bits. Therefore, retry with last = free; from free -> old last
		// NOTE: Not implementing currently to keep simplicity.
		
		return 0;
	}
	
	found:
	  // No free bit was found below here, so update last to a new best guess.
	  last = ptr;
	
	  // This simply uses BSF to load the index of the lowest 1-bit into slot,
	  // and after that resets that same bit using LEA and AND: *ptr &= *ptr - 1;
	  __asm__ (
	            "lea	%2, [%1 - 1]" "\n\t"
	            "bsf	%0, %1"       "\n\t"
				"and	%1, %2"
	           : "=r" (slot), "+r" (*ptr), "=r" (temp)
	           : /* no sole inputs */
	           : "cc"
	          );
	
	  slot += (ptr - free) / sizeof(msfree_t);
	
	return slot;
}

// ------------------------------------------------------------------------- //

template <class T>
void mstore<T>::deallocate(size_t slot)
{
	msfree_t *ptr = free + (slot >> MS_SHIFT);
	
	if (ptr < last)
		last = ptr;
	
	__asm__ (
	         "bts	%0, %1"
	         : "+rm" (*ptr)
	         : "ri" (slot & MS_MASK)
	         : "cc"
	        );
}

// ------------------------------------------------------------------------- //

template <class T>
void mstore<T>::iterate (void (*function) (T *object_ptr, void *argument), void *argument)
{
	T        *data_ptr = data;
	msfree_t *free_ptr = free;
	
	size_t count = fsize;
	
	msfree_t temp1, temp2; // temp2 is only used for the intermediate assembly calculation.
	msfree_t bitslot;
	
	// count serves to let us loop across exactly the size of the free array.
	while (count--)
	{
		temp1 = ~(*free_ptr);
		
		while (temp1)
		{
			// This simply uses BSF to load the index of the lowest 1-bit into bitslot,
			// and after that resets that same bit using LEA and AND: temp1 &= temp1 - 1;
			__asm__ (
			         "lea	%2, [%1 - 1]" "\n\t"
			         "bsf	%0, %1"       "\n\t"
			         "and	%1, %2"
			         : "=D" (bitslot), "+r" (temp1), "=r" (temp2) // bitslot is kept in EDI/RDI.
			         : /* no sole inputs */
			         : "cc"
			        );
			
			// Call the given function, giving it the pointer of the object and the given argument.
			function((data_ptr + bitslot), argument);
		}
		
		// data_ptr is like (data + slot*32 (or 64)), so by simply adding a bitslot to it,
		// we retrieve a pointer to the object. Hence every iteration, 32 (or 64) is added.
		free_ptr++;
		data_ptr += (sizeof(msfree_t) * CHAR_BIT);
	}
}

#endif // UTILS_MSTORE_HPP_INCLUDED
