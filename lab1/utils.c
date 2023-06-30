#include <stdlib.h>
#include "utils.h"
#include "lc3bassemblerdefines.h"

// Convert register number string to decimal number
unsigned int regnum(char *reg)
{
    int num = 0;
    int i = 0;
    while (*(reg + i))
    {
        num = num * 10 + (*(reg + i) - '0');
        ++i;
    }

    if (num > 7)
        ERROR("ERROR : Invalid Register Operand\n");

    return num;
}

// Convert uppercase character to lowercase character
void my_tolower(char *c)
{
    if (*c >= 65 && *c <= 90)
        *c += 32;
}

// Convert string to decimal number
int my_atoi(const char* str)
{
    int neg = 1;
    int i = 0;

    if (*str == '-')
    {
        neg = -1;
        ++i;
    }

    int num = 0;
    while (*(str + i))
    {
        if (*(str + i) >= '0' && *(str + i) <= '9')
            num = num * 10 + (*(str + i) - '0');
        else
        {
            ERROR("ERROR: Constant is not decimal\n");
            exit(3);
        }
        ++i;
    }

    return neg * num;
}

// Convert hex string to decimal number
int hex2i(const char* str)
{
    int neg = 1;
    int i = 0;

    if (*str == '-')
    {
        neg = -1;
        ++i;
    }

    int num = 0;
    while(*(str + i))
    {
        if (*(str + i) >= '0' && *(str + i) <= '9')
        {
            num = num * 16 + (*(str + i) - '0');
            ++i;
        }
        else if (*(str + i) >= 'a' && *(str + i) <= 'f')
        {
            num = num * 16 + (*(str + i) - 'a') + 10;
            ++i;
        }
        else
        {
            ERROR("ERROR: Constant is not hexadecimal\n");
            exit(3);
        }
    }

    return neg * num;
}
