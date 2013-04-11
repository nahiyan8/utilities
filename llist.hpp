#ifndef LLIST_HPP_INCLUDED
#define LLIST_HPP_INCLUDED

/****************************************************************\
* @note : A sentinel node is used, which uses a special struct.
*
* @note : *pTemp1 is used to traverse the list, while *pTemp2
*           is usually used to do some temporary things.
*
* @note : nUsable is just nNodes - 1. It's given to the user so
*           the user can loop and such.
*
* @note : privAlloc()'s allocated nodes start from startSlot.
*
* @note : privDealloc() deallocates the specified nodes and all
*           nodes in between.
*
* @note : privAlloc() returns the last node allocated, or if
*           no nodes were allocated, will return &[startSlot - 1]
*
* @todo : Have tempSlot at the slot-1, most functions will
*	    need slot-1, currently they have to re-traverse.
*
* @todo : Remove pTempN from class, initialize seperately.
*
* @todo : Change to *data, make function allocate in chunks.
\****************************************************************/

#define TRAVERSE(slot) if (tempSlot > slot) {pTemp1 = start; tempSlot = 0;} for (; tempSlot < slot; tempSlot++) pTemp1 = pTemp1->next;

template <class type>
class llist
{
    protected:
        // A node! Usually I'd put this outside but eh, what the hell.
        struct node
        {
            node *next;
            type data;
        };

        // The setinel node! This specialization is because the data member might not be worth having.
        struct setinelNode
            { node *next; };

	node *start, *pTemp1, *pTemp2;
        //node *start, *current;
	uint64_t nNodes, nUsable, tempSlot;

        // Allocation and deallocation
        node* privAlloc( uint64_t startSlot, uint64_t newNodes )
        {
            // Incase the user wants to allocate starting from somewhere not yet allocated, allocate nodes upto startSlot.
            if ( startSlot > nNodes )
                { newNodes += (startSlot - nNodes); startSlot = nNodes; }

            // Decremented because we want to go to the specified node's previous one.
            startSlot--;

            // If tempSlot < startSlot, it must be >= start, so traverse from temp. If tempSlot > startSlot, traverse from start;
            if ( tempSlot > startSlot )
                { pTemp1 = start; tempSlot = 0; }

            // Traverse!! (There's a macro declared as TRAVERSE(slot) that does this and the previous if, since traversion is done a lot.)
            for ( ; tempSlot < startSlot; tempSlot++ )
                pTemp1 = pTemp1->next;

            // To link the newly made nodes to the rest of the list later.
            pTemp2 = pTemp1->next;

            // Allocate
            while ( newNodes-- != 0 )
            {
                try
                    { pTemp1->next = new node; }
                catch ( ... )
                    { pTemp1->next = pTemp2; nUsable = nNodes; nUsable--; return false; }

                pTemp1 = pTemp1->next;

                tempSlot++;
                nNodes++;
            }

            // Reconnect the list..
            pTemp1->next = pTemp2;

            // For user needs. :)
            nUsable = nNodes; nUsable--;

            return pTemp1;
        };

        void privDealloc( uint64_t from, uint64_t to )
        {
            // Check if user wants to deallocate the far lands.
            if ( from >= nNodes || to >= nNodes )
                return;

            // to should be > from, if not, then swap!
            if ( to < from )
                { uint64_t swap = to; to = from; from = swap; }

            // TRAVERSAL! (We want to deallocate from behind)
            from--; to--;
            TRAVERSE( from );

            // They come, and be de-allo-cated, and be banished, like anni-hil-ated.
            while ( tempSlot < to )
            {
                pTemp2 = pTemp1->next;
                pTemp1->next = pTemp1->next->next;
                delete pTemp2;

                tempSlot++;
                nNodes--;
            }

            // We sorta used tempSlot a way not meant for it. Fixing it..
            tempSlot = from;
            nUsable = nNodes; nUsable--;
        };

    public:
        // Constructors are constructing some constructs, in that construction op.
        llist()
        {
            start = reinterpret_cast <node*> (new setinelNode); pTemp1 = start; start->next = 0;
            nNodes = 1; nUsable = 0; tempSlot = 0;
        };

        llist( uint32_t newNodes )
        {
            start = reinterpret_cast <node*> (new setinelNode); pTemp1 = start; start->next = 0;
            nNodes = 1; nUsable = 0; tempSlot = 0;

            privAlloc( 1, newNodes );
        };

        // Destructors are using thermite!
        ~llist()
            { privDealloc(1, nUsable); delete ((setinelNode*) start); };

        // Allocation and deallocation candy wrappers! (to prevent setinel node access/modification.)
        inline type& alloc( uint64_t startSlot, uint64_t newNodes )
            { startSlot++; return (privAlloc(startSlot, newNodes))->data; };

        inline void dealloc( uint64_t from, uint64_t to )
            { from++; to++; privDealloc(from, to); };

        // Access
        type& operator[]( uint64_t slot )
        {
            // For le setinel node
            slot++;

            // Re-use, optimizo!
            if ( tempSlot == slot )
                { return pTemp1->data; }

            TRAVERSE( slot );
            return pTemp1->data;
        };

        /* COPY DEM DATAS - You're right! <reply> You're left! TODO: *this
        llist <type> & operator=( llist <type> &from )
        {
            pTemp1 = start->next; pTemp2 = from.start->next;
            tempSlot = 1;

            for ( ; tempSlot < from.nNodes; tempSlot++ )
            {
                // Not allocated, so allocate!
                if ( tempSlot == nNodes )
                    { pTemp1->next = new node; nNodes++; }

                // JUMP FOOOOR IIIIT!!
                pTemp1 = pTemp1->next;


            }
        };
        */

        // I really think you should swap yourself.
        void swap( uint64_t nodeA, uint64_t nodeB )
        {
            // I don't think we have that..
            if ( nodeA >= nUsable || nodeB >= nUsable )
                return;

            // We want to go to the lower one first (nodeA).. NONSENSE!! And return if A == B because.. I think it's self-explanatory :)
            if ( nodeB > nodeA )
                { uint64_t transfer = nodeA; nodeA = nodeB; nodeB = transfer; }
            else
                if ( nodeA == nodeB )
                    return;

            // We want to go to the nodes' previous node, so don't add for the setinel node!

            // Traverse to nodeA..
            TRAVERSE( nodeA );
            pTemp2 = pTemp1;

            // Now to nodeB.. TRAVERSE() wasn't used because it checks if the node we're traveling to is lower than pTemp1, which was already done.
            for ( ; tempSlot < nodeB; tempSlot++ )
                pTemp1 = pTemp1->next;

            // SWAP THEM THINGIES!!
            node *swapper = pTemp1->next;
            pTemp1->next = pTemp2->next;
            pTemp2->next = swapper;
        }

        // Miscellany
        inline type& alloc( uint64_t newNodes )
            { return (privAlloc(nNodes, newNodes))->data; };

        inline void dealloc( uint64_t slot )
            { slot++; privDealloc(slot, slot); };

        inline uint64_t size()
            { return nUsable; };

        type* operator->()
            { return &(start->next->data); }
};

#endif
