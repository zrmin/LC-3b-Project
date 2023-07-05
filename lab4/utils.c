#include "utils.h"
#include <stdio.h>

int signExt(int num, int bitNum)
{
    printf("Beginging signExt\n");
    printf("original NUMBER = %d\n", num);

    if (num >> (bitNum - 1) & 0x1)
    {
        int amount = 8 * sizeof(int) - bitNum;
        printf("amount = %d\n", amount);
        num = num << amount;
        printf("num << amount = %x", num);
        num = num >> amount;
        printf("num = %d\n", num);
    }

    return num;
}
