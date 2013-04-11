/*************************************************************\
* @brief : Set an array of data to another array of data, of
*          of the length specified.
*
* @param - type *to : The address to where the data is being
*                     transfered.
* @param - type *from : The address to where the data is being
*                       transfered from.
* @param - uint8_t length : The length of the data being
*                           transfered.
\*************************************************************/
template <typename type>
void mov( type *to, type *from, size_t length )
{
    while ( length-- != 0 )
        *(to++) = *(from++);
}

/*************************************************************\
* @brief : Counts the length of a character string.
*
* @param - char *string : A pointer to a null-terminated
*                         string of characters.
*
* @return - uint8_t : Length of given string, in characters.
\*************************************************************/
uint16_t length( char *string )
{
    uint8_t length = 0;

    while ( *string != '\0' )
        { length++; string++; }

    return length;
}

/*************************************************************\
* @brief : Compares 2 strings.
\*************************************************************/
bool compare( char* str1, char* str2 )
{
    while ( *str1 != '\0' || *str2 != '\0' )
        if ( *(str1++) != *(str2++) )
            return false;

    return true;
}

/*************************************************************\
* @brief : Finds a particular string within a string.
*
* @param - char *string : Where to search (set, haystack)
* @param - char *subString : What to search (subset, needle)
*
* @return - uint16_t : The slot after the found string, if not
*                      found, will return ~0 (0xFFFF).
*
* @note : The function can be made to find the slot where the
*         tofind starts, by changing the return from
*         (slot + findSlot) to just (slot).
*
* @fixme : Doesn't work when string == tofind
* @fixme : Need to optimise (e.g. scan string for the first
*          letter of subString.
\*************************************************************/
uint16_t find( const char *string, const char *subString )
{
    uint16_t slot = 0, subSlot;

    while ( *string != '\0' )
    {
        for ( subSlot = 0; string[subSlot] == subString[subSlot]; subSlot++ );

        if ( subString[subSlot] == '\0' )
            return slot + subSlot;

        string++; slot++;
    }

    return ~0;
}

/*************************************************************\
* @brief : Gets a parameter/option from a string. Useful for
*          the win32 main(). Otherwise, not useful in *nix.
\*************************************************************/
bool getStrParam( char *param, char *output )
{
    static uint32_t paramSlot = 0;
    uint32_t strSlot = 0;
    char terminator = ' ';

    if (paramSlot) paramSlot++;
    if (param[paramSlot] == '\"') {terminator = '\"'; paramSlot++;}

    for ( ; param[paramSlot] != terminator && param[paramSlot] != '\0'; output[strSlot++] = param[paramSlot++] )

    if ( param[paramSlot] == '\0' && strSlot == 0 ) return false;
    if ( terminator == '\"' ) paramSlot++;

    output[strSlot] = '\0';
    return true;
}

/*************************************************************\
* @brief : Gets the square-root of a given number n.
* @fixme : May get stuck in an infinite loop.
\*************************************************************/
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
