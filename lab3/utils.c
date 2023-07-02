#include "utils.h"

int signExt(int num, int bitNum)
{
    if (num >> (bitNum - 1) & 0x1)
    {
        int amount = 8 * sizof(int) - bitNum;
        num = num << amount;
        num = num >> amount;
    }

    return num;
}
