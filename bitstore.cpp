#include "bitstore.hpp"

// Definitions
typedef unsigned char byte;
typedef unsigned long long uint64_t;

// Allocation.
void bitstore::alloc( uint64_t bits, bool value )
{
    uint64_t bytes = (bits >> 3) + ((bits & 7) != 0 ? 1 : 0), newSize = size + bytes;
    byte setVal = value == true ? 0xFF : 0x00;

    array = new byte [bytes];

    for ( uint64_t slot = size; slot < newSize; )
        { array[slot] = setVal; slot++; }

    size = newSize;
}

// Deallocation.
void bitstore::dealloc( uint64_t bits )
{
    uint64_t bytes = (bits >> 3) + ((bits & 7) != 0 ? 1 : 0), newSize = size - bytes;

    for ( uint64_t slot = size; slot > newSize; )
        { delete array; slot--; }

    size = newSize;
}

/* Single-bit Access and Modification */
bool bitstore::get( register uint64_t slot )
{
    return (array[slot >> 3] >> (slot & 7)) & 0x1;
}

void bitstore::set( register uint64_t slot, register bool value )
{
    if ( value == true )
        { array[slot >> 3] |= (0x1 << (slot & 7)); return; }
    else
        { array[slot >> 3] &= ~(0x1 << (slot & 7)); return; }
}

void bitstore::setOn( register uint64_t slot )
{
    array[slot >> 3] |= 0x1 << (slot & 7);
}

void bitstore::setOff( register uint64_t slot )
{
    array[slot >> 3] &= ~(0x1 << (slot & 7));
}

void bitstore::flip( register uint64_t slot )
{
    array[slot >> 3] ^= 0x1 << (slot & 7);
}

/* Multi-bit Access and Modification */
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
    byte setVal = value == true ? 0xFF : 0x00;

    for ( uint64_t slot = 0; slot < size; slot++ )
        array[slot] = setVal;
}
