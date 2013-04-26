#ifndef LLIST_HPP_INCLUDED
#define LLIST_HPP_INCLUDED

/****************************************************************\
* @note : A sentinel node is used, which uses a special struct.
*
* @note : *current is used to traverse the list, while *temp
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
* @todo : Have currentSlot at the slot-1, most functions will
*	    need slot-1, currently they have to re-traverse.
*
* @todo : Remove pTempN from class, initialize seperately.
*
* @todo : Change to *data, make function allocate in chunks.
\****************************************************************/

#define TRAVERSE(slot) if (currentSlot > slot) {current = start; currentSlot = 0;} for (; currentSlot < slot; currentSlot++) current = current->next;

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

		// The setinel node! Because we don't need data here, nor will be able to use it.
		struct setinelNode
			{ node *next; };

		node *start, *current, *temp;
		//node *start, *current, *temp;
		unsigned int nNodes, nUsable, currentSlot;

        // Allocation and deallocation
        node* privAlloc( unsigned int startSlot, unsigned int newNodes )
        {
            // Incase the user wants to allocate starting from somewhere not yet allocated, allocate nodes upto startSlot.
            if ( startSlot > nNodes )
                { newNodes += (startSlot - nNodes); startSlot = nNodes; }

            // Decremented because we want to go to the specified node's previous one.
            startSlot--;

            // If currentSlot < startSlot, it must be >= start, so traverse from pointer. If currentSlot > startSlot, traverse from start;
            if ( currentSlot > startSlot )
                { current = start; currentSlot = 0; }

            // Traverse!! (There's a macro declared as TRAVERSE(slot) that does this and the previous if, since traversion is done a lot.)
            for ( ; currentSlot < startSlot; currentSlot++ )
                current = current->next;

            // To reconnect the newly made nodes to the rest of the list later.
            temp = current->next;

            // Allocate, move to newly allocated node, change relevant integers.
            while ( newNodes-- ) // != 0
			{
				// Allocate a new node, while being wary of exceptions.
				try 
					{ current->next = new node; }
				// Exception caught, could not allocate another node. Reconnect the list, update list size and return false.
				// @fixme: the return type is node*, don't return false. Choose another, wisely this time, check for good ideas.. or maybe fail..
				catch ( ... )
					{ current->next = temp; nUsable = nNodes - 1; return false; }

				// Change to the newly allocated node.
                current = current->next;
		
				// Update relevant integers.
                currentSlot++;
                nNodes++;
            }

            // Reconnect the list..
            current->next = temp;

            nUsable = nNodes - 1;

            return current;
        };

        void privDealloc( unsigned int from, unsigned int to )
        {
            // One does not simply deallocate the farlands.
            if ( from >= nNodes || to >= nNodes )
                return;

            // Should be (to > from), if not, swap!
            if ( to < from )
                { unsigned int swap = to; to = from; from = swap; }

            // TRAVERSAL! (We want to deallocate from behind)
            from--; to--;
            TRAVERSE( from );

            // They come, and be de-allo-cated, and be banished, like anni-hil-ated.
            while ( currentSlot < to )
            {
                temp = current->next;
                current->next = current->next->next;
                delete temp;

                currentSlot++;
                nNodes--;
            }

            // We sorta used currentSlot a way not meant for it. Fixing it..
            currentSlot = from;
            nUsable = nNodes; nUsable--;
        };

    public:
        // Constructors are constructing some constructs, in that construction op.
        llist()
        {
            start = reinterpret_cast <node*> (new setinelNode); current = start; start->next = 0;
            nNodes = 1; nUsable = 0; currentSlot = 0;
        };

        llist( unsigned int newNodes )
        {
            start = reinterpret_cast <node*> (new setinelNode); current = start; start->next = 0;
            nNodes = 1; nUsable = 0; currentSlot = 0;

            privAlloc( 1, newNodes );
        };

        // Destructors are using thermite!
        ~llist()
            { privDealloc(1, nUsable); delete ((setinelNode*) start); };

        // Allocation and deallocation candy wrappers! (to prevent setinel node access)
        inline type& alloc( unsigned int startSlot, unsigned int newNodes )
            { startSlot++; return (privAlloc(startSlot, newNodes))->data; };

        inline void dealloc( unsigned int from, unsigned int to )
            { from++; to++; privDealloc(from, to); };

        // Access
        type& operator[]( unsigned int slot )
        {
            // For le setinel node
            slot++;

            // Re-use, optimizo!
            if ( currentSlot == slot )
                { return current->data; }

            TRAVERSE( slot );
            return current->data;
        };

        /* COPY DEM DATAS - You're right! <reply> You're left! TODO: *this
        llist <type> & operator=( llist <type> &from )
        {
            current = start->next; temp = from.start->next;
            currentSlot = 1;

            for ( ; currentSlot < from.nNodes; currentSlot++ )
            {
                // Not allocated, so allocate!
                if ( currentSlot == nNodes )
                    { current->next = new node; nNodes++; }

                // JUMP FOOOOR IIIIT!!
                current = current->next;


            }
        };
        */

        // I really think you should swap yourself.
        void swap( unsigned int nodeA, unsigned int nodeB )
        {
            // I don't think we have that..
            if ( nodeA >= nUsable || nodeB >= nUsable )
                return;

            // We want to go to the lower one first (nodeA).. NONSENSE!! And return if A == B because.. I think it's self-explanatory :)
            if ( nodeB > nodeA )
                { unsigned int transfer = nodeA; nodeA = nodeB; nodeB = transfer; }
            else
                if ( nodeA == nodeB )
                    return;

            // We want to go to the nodes' previous node, so don't add for the setinel node!

            // Traverse to nodeA..
            TRAVERSE( nodeA );
            temp = current;

            // Now to nodeB.. TRAVERSE() wasn't used because it checks if the node we're traveling to is lower than current, which was already done. </optimised>
            for ( ; currentSlot < nodeB; currentSlot++ )
                current = current->next;

            // SWAP THEM THINGIES!!
            node *swapper = current->next;
            current->next = temp->next;
            temp->next = swapper;
        }

        // Miscellany
        inline type& alloc( unsigned int newNodes )
            { return (privAlloc(nNodes, newNodes))->data; };

        inline void dealloc( unsigned int slot )
            { slot++; privDealloc(slot, slot); };

        inline unsigned int size()
            { return nUsable; };

		// Upcoming!
//		inline unsigned int truesize()
//			{ return nNodes; };

		type* operator->()
            { return &(start->next->data); }
};

#endif
