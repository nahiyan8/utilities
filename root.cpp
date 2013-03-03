long double root( uint64_t n )
{
    long double nRoot[2] = {1, 0};
    bool slot = 0, newSlot = 1;

    nRoot[0] = (n >> 1) + (n & 1);

    while ( nRoot[0] != nRoot[1] )
    {
        nRoot[newSlot] = (nRoot[slot] + (n / nRoot[slot])) / 2;
        newSlot = slot; slot ^= 0x1;
    }

    return *nRoot;
}
