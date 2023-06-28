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
