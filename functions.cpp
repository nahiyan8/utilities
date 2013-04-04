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
* @param - char *string : The string where tofind is being
*                         searched.
* @param - char *tofind : The string which the function is
*                         looking for.
*
* @return - uint16_t : The slot after the found string, if not
*                      found, will return 0xFFFF.
*
* @note : The function can be made to find the slot where the
*         tofind starts, by changing the return from
*         (slot + findSlot) to just (slot).
*
* @fixme (Nahiyan#5#): Doesn't work when string == tofind
\*************************************************************/
uint16_t find( char *string, char *tofind )
{
    uint16_t slot = 0, findSlot;

    while ( *string != '\0' )
    {
        for ( findSlot = 0; string[findSlot] == tofind[findSlot]; findSlot++ );

        if ( tofind[findSlot] == '\0' )
            return slot + findSlot;

        string++; slot++;
    }

    return ~0;
}

/*************************************************************\
* @brief : Gets a parameter/option from a string.
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
* @fixme : May be stuck in an infinite loop.
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
