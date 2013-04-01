#ifndef BITSTORE_H
#define BITSTORE_H

typedef unsigned char byte;
typedef unsigned long long uint64_t;

class bitstore
{
    public:
        byte *array;
        uint64_t size;

        // Constructors
        bitstore()
            { array = 0; size = 0; };

        bitstore( uint64_t bits, bool value )
            { size = 0; alloc( bits, value ); };

        bitstore( byte *pointer )
            { array = pointer; size = 0; };

        // Destructor
        ~bitstore()
            { delete [] array; };

        // Allocation and Deallocation
        void alloc  ( uint64_t bits, bool value );
        void dealloc( uint64_t bits );

        // Single-bit Access and Modification
        bool get( register uint64_t slot );
        void set( register uint64_t slot, register bool value );

        void setOn ( register uint64_t slot );
        void setOff( register uint64_t slot );

        void flip( register uint64_t slot );

        // Multi-bit Access and Modification
        uint64_t get( register uint64_t slot, register byte bits );
        void     set( register uint64_t slot, register byte bits, register uint64_t value );

        void setOn ( register uint64_t slot, register uint64_t bits );
        void setOff( register uint64_t slot, register uint64_t bits );

        void flip( register uint64_t slot, register uint64_t bits );

        // Reset
        void setAll( bool value );

        // Some other overloads..
        inline bool operator[]( register uint64_t slot )
            { return get(slot); };
};

#endif // BITSTORE_H
