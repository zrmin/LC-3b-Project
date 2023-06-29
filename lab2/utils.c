#include "utils.h"

int signExt5(const int value)
{
    // Get sign bit
    int signBit = (value >> 4) & 0x1;

    // Sign extension
    if (signBit)
    {
        return ((value & 0xF) | 0xFFFFFFF0);
    }

    return value;
}

int signExt6(const int value)
{
    // Get sign bit
    int signBit = (value >> 5) & 0x1;

    // Sign extension
    if (signBit)
    {
        return ((value & 0x1F) | 0xFFFFFFE0);
    }

    return value;
}

int signExt8(const int value)
{
    // Get sign bit
    int signBit = (value >> 7) & 0x1;

    // Sign extension
    if (signBit)
    {
        return ((value & 0x7F) | 0xFFFFFF80);
    }

    return value;
}

int signExt9(const int value)
{
    // Get sign bit
    int signBit = (value >> 8) & 0x1;

    // Sign extension
    if (signBit)
    {
        return ((value & 0xFF) | 0xFFFFFF00);
    }

    return value;
}


int signExt11(const int value)
{
    // Get sign bit
    int signBit = (value >> 10) & 0x1;

    // Sign extension
    if (signBit)
    {
        return ((value & 0x3FF) | 0xFFFFFC00);
    }

    return value;
}

int hex2i(const char *str)
{
    int num = 0;
    int i = 0;
    while (*(str + i))
    {
        if (*(str + i) >= '0' && *(str + i) <= '9')
        {
            num = num * 16 + (*(str + i) - '0');
            ++i;
        }

        if (*(str + i) >= 'a' && *(str + i) <= 'f')
        {
            num = num * 16 + (*(str + i) - 'a') + 10;
            ++i;
        }
    }

    return num;
}

int my_atoi(const char* str)
{
    int num = 0;
    int i = 0;

    while (*(str + i))
    {
        if (*(str + i) >= '0' && *(str + i) <= '9')
        {
            num = num * 10 + (*(str + i) - '0');
            ++i;
        }
    }

    return num;
}

int RSHFA(int num, const int amount4)
{
    int i = 0;
    int mask = 0x8000;
    int signBit = (num >> 15) & 0x1;

    if (signBit)
    {
        // Right shift logical
        num = num >> amount4;
        for (i = 0; i < amount4; ++i)
        {
            num = num | mask;
            mask = mask >> 1;
        }
    }
    else
    {
        num = num >> amount4;
    }

    return num;
}

/*
void setcc(const int TEMP_DR, struct System_Latches* CURRENT_LATCHES)
{
    if (TEMP_DR == 1)
    {
        *CURRENT_LATCHES.N = 0;
        *CURRENT_LATCHES.Z = 0;
        *CURRENT_LATCHES.P = 1;
    }
    else if (TMEP_DR == 0)
    {
        *CURRENT_LATCHES.N = 0; 
        *CURRENT_LATCHES.Z = 1;
        *CURRENT_LATCHES.P = 0;
    }
    else
    {
        *CURRENT_LATCHES.N = 1;
        *CURRENT_LATCHES.Z = 0;
        *CURRENT_LATCHES.P = 0;
    }
}
*/
