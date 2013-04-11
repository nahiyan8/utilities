#include "bitstore.hpp"

/*
	I MAY have gone a little overboard with the register keyword..
	Some keypoints to help you read this:
		(bits >> 3) is bits in bytes.
		(bits & 7) is the leftover bits. (bytes + leftover_bits == bits)
*/

// Definitions
typedef unsigned char byte;
typedef unsigned long long uint64_t;

// Allocation.
void bitstore::alloc( uint64_t bits, bool value )
{
    uint64_t byteSize = (bits >> 3) + ((bits & 7) ? 1 : 0), newSize = size + byteSize;
    byte setValue = value ? 0xFF : 0x00;

    array = new byte [byteSize];

    for ( uint64_t slot = size; slot < newSize; )
        { array[slot] = setValue; slot++; }

    size = newSize;
}

/* Deallocation, I was new, I didn't know how it worked :P I'll make it work later, with the llist class probably.. */

/* Single-bit functions */
bool bitstore::get( register uint64_t slot )
{
    return (array[slot >> 3] >> (slot & 7)) & 0x1;
}

void bitstore::set( register uint64_t slot, register bool value )
{
    // This is basically setOn or setOff depending on the value..
    if ( value == true )
        { array[slot >> 3] |= (0x1 << (slot & 7)); return; }
    else
        { array[slot >> 3] &= ~(0x1 << (slot & 7)); return; }
}

void bitstore::setOn( register uint64_t slot )
{
    // OR the relevant byte with a 0x1 shifted to where the bit should be.
    array[slot >> 3] |= 0x1 << (slot & 7);
}

void bitstore::setOff( register uint64_t slot )
{
    // AND the relevant byte with all 1s, but 0 for the actual bit.
    array[slot >> 3] &= ~(0x1 << (slot & 7));
}

void bitstore::flip( register uint64_t slot )
{
    // XOR the relevant byte by 0x1 shifted to where the bit should be.. I feel like I'm copy-pasting..
    array[slot >> 3] ^= 0x1 << (slot & 7);
}

/* Multi-bit - I'll add (probably helpful) comments to these later.. when I figure them out.. */
uint64_t bitstore::get( register uint64_t slot, register byte bits )
{
    return (*((uint64_t*) (array + (slot >> 3))) >> (slot & 7)) & ((0x1 << bits) - 1);
}

void bitstore::set( register uint64_t slot, register byte bits, register uint64_t value )
{
    uint64_t *pointer = (uint64_t*) (array + (slot >> 3));
    byte arrayBits = slot & 7;

    *pointer &= ~(((0x1 << bits) - 1) << arrayBits);
    *pointer |= (value << arrayBits);
}

void bitstore::setOn ( register uint64_t slot, register uint64_t bits )
{
    *((uint64_t*) (array + (slot >> 3))) |= ((0x1 << bits) - 1) << (slot & 7);
}

void bitstore::setOff( register uint64_t slot, register uint64_t bits )
{
    *((uint64_t*) (array + (slot >> 3))) &= ~(uint64_t ((0x1 << bits) - 1) << (slot & 7));
}

void bitstore::flip( register uint64_t slot, register uint64_t bits )
{
    *((uint64_t*) (array + (slot >> 3))) ^= ((0x1 << bits) - 1) << (slot & 7);
}

/* Reset */
void bitstore::setAll( bool value )
{
    byte setValue = value ? 0xFF : 0x00;

    for ( uint64_t slot = 0; slot < size; slot++ )
        array[slot] = setValue;
}
