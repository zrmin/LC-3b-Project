#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lc3bassemblerdefines.h"
#include "utils.h"

// Symbol Table
typedef struct
{
    unsigned short address;
    char label[MAX_SYMBOL_LENGTH + 1];
} symbolEntry;

symbolEntry symbolTable[MAX_SYMBOLS];

char* tokArray[6] = {NULL};

int foundOrig = 0;
unsigned short baseAddress = 0;
FILE* inFile = NULL;
FILE* oFile = NULL;
char line[MAX_LENGTH + 1];


// Parse an instruction to tokens
// tokens: label, opcode, arg1, arg2, arg3, arg4
char cacheLine[MAX_LENGTH + 1] = MYREGRET(1617GAOMENG);
void parse(void)
{
    char *ptr = fgets(line, MAX_LENGTH, inFile);

    // Checking if there's a missing '.end' directive
    if (ptr == NULL)
    {
        ERROR("MISSING .END DIRECTIVE!!!\n");
        exit(4);
    }

    strcpy(cacheLine, line);

    // Convert to lowercase
    int i = 0;
    while(line[i])
    {
        my_tolower(&line[i]);
        ++i;
    }

    // Ignore comments
    i = 0;

    while(line[i] != ';' && line[i] != '\0' && line[i] != '\n')
    {
        ++i;
    }
    line[i] = '\0';

    // Parse the line into tokens
    tokArray[0] = strtok(line, "\n\t\r ,");

    i = 1;
    while ((tokArray[i-1] != NULL) && (i < 6))
    {
        tokArray[i] = strtok(NULL, "\n\t\r ,");
        ++i;
    }

    i = 0;
    while((tokArray[i] != NULL) && (i < 6))
    {
        ++i;
    }

    while(i < 6)
    {
        tokArray[i] = NULL;
        ++i;
    }

    // If this is an empty line, parse the next line
    if (!tokArray[0])
    {
        parse();
    }
}

int tokToValue(const char* tok)
{
    if (!strcmp(tok, "add"))
        return 1;

    if (!strcmp(tok, "and"))
        return 5;

    if (!strcmp(tok, "br") || !strcmp(tok, "brp") || !strcmp(tok, "brz") 
            || !strcmp(tok, "brzp") || !strcmp(tok, "brn")
            || !strcmp(tok, "brnp") || !strcmp(tok, "brnz")
            || !strcmp(tok, "brnzp"))
        return 0;

    if (!strcmp(tok, "jmp") || !strcmp(tok, "ret"))
        return 12;

    if (!strcmp(tok, "jsr") || !strcmp(tok, "jsrr"))
        return 4;

    if (!strcmp(tok, "ldb"))
        return 2;

    if (!strcmp(tok, "ldw"))
        return 6;

    if (!strcmp(tok, "lea"))
        return 14;

    if (!strcmp(tok, "rti"))
        return 8;

    if (!strcmp(tok, "lshf") || !strcmp(tok, "rshfl") || !strcmp(tok, "rshfa"))
        return 13;

    if (!strcmp(tok, "stb"))
        return 3;

    if (!strcmp(tok, "stw"))
        return 7;

    if (!strcmp(tok, "trap") || !strcmp(tok, "halt"))
        return 15;

    if (!strcmp(tok, "not") || !strcmp(tok, "xor"))
        return 9;

    if (!strcmp(tok, ".orig"))
        return -2;

    if (!strcmp(tok, ".end"))
        return -3;

    if (!strcmp(tok, ".fill"))
        return -4;

    if (!strcmp(tok, "nop"))
        return -5;

    if (*tok == '\0')
        return -6;

    // Label
    return -1;
}

// Process baseAddress in order to caculate label's address
void BaseAddress(void)
{
    // Error checking
    if (foundOrig)
    {
        ERROR("ERROR: .orig can only exist once!\n");
        exit(4);
    }

    int i = 0;
    // Decimal number
    if (tokArray[1][i] == '#')
    {
        ++i;
        while (tokArray[1][i] != '\0')
        {
            baseAddress = 10 * baseAddress + (tokArray[1][i] - '0');
            ++i;
        }
    }

    // Hex number
    if (tokArray[1][i] == 'x')
    {
        ++i;
        baseAddress = hex2i(&tokArray[1][i]);
    }

    // Error checking
    if (baseAddress < 0 || baseAddress > 65535)
    {
        ERROR("baseAddress exceed 0-65535\n");
        exit(3);
    }

    if (baseAddress % 2)
    {
        ERROR("baseAddress must be word aligned!\n");
        exit(3);
    }

    foundOrig = 1;
}

// Convert instruction to machine code
// Function prototype
unsigned short add();
unsigned short and();
unsigned short br(const unsigned short lineAddress);
unsigned short jmp_ret();
unsigned short jsr_jsrr(const unsigned short lineAddress);
unsigned short ldb();
unsigned short ldw();
unsigned short lea(const unsigned short lineAddress);
unsigned short rti();
unsigned short shf();
unsigned short stb();
unsigned short stw();
unsigned short trap_halt();
unsigned short not_xor();
unsigned short orig();
unsigned short fill();
unsigned short nop();

unsigned short convert(const int tokValue, const short lineAddress)
{
    switch (tokValue) {
        case 1:
            return add();
        case 5:
            return and();
        case 0:
            return br(lineAddress);
        case 12:
            return jmp_ret();
        case 4:
            return jsr_jsrr(lineAddress);
        case 2:
            return ldb();
        case 6:
            return ldw();
        case 14:
            return lea(lineAddress);
        case 8:
            return rti();
        case 13:
            return shf();
        case 3:
            return stb();
        case 7:
            return stw();
        case 15:
            return trap_halt();
        case 9:
            return not_xor();
        case -2:
            return orig();
        case -4:
            return fill();
        case -5:
            return nop();
        default:
            return 0xffff;
    }
}

unsigned short add(void)
{
    int DR = 0, SR1 = 0, SR2 = 0, imm5 = 0;

    // Operand error checking
    if(!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (ADD): Missing Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
    {
        ERROR("ERROR (ADD): Invalid operand\n");
        exit(4);
    }

    if (tokArray[3][0] != 'r' && tokArray[3][0] != 'x' && tokArray[3][0] != '#')
    {
        ERROR("ERROR (ADD): Invalid Operand\n");
        exit(4);
    }

    // Get DR
    DR = regnum(&tokArray[1][1]);

    // Get SR1
    SR1 = regnum(&tokArray[2][1]);

    // Get Operand2: register or imm
    if (tokArray[3][0] == 'r') // Operand2 is SR2
    {
        SR2 = regnum(&tokArray[3][1]);
        return ((1 << 12) | (DR << 9) | (SR1 << 6) | (SR2));
    }
    else // Operand2 is imm5
    {
        int i = 0;
        if (tokArray[3][i] == '#')
        {
            imm5 = my_atoi(&tokArray[3][i + 1]);
        }

        if (tokArray[3][i] == 'x')
        {
            imm5 = hex2i(&tokArray[3][i + 1]);
        }

        // Imm5 error checking
        if (imm5 > 15 || imm5 < -16)
        {
            ERROR("ERROR (ADD): Invalid constant\n");
            exit(3);
        }

        imm5 = imm5 & 0x1F;
        return ((1 << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | (imm5));
    }
}

unsigned short and(void)
{
    int DR = 0, SR1 = 0, SR2 = 0, imm5 = 0;

    // Operands error checking
    // AND DR, SR1, SR2(imm5)
    if (!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (AND): missing operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
    {
        ERROR("ERROR (AND): Invalid Register Operand\n");
        exit(4);
    }

    if (tokArray[3][0] != 'r' && tokArray[3][0] != '#' && tokArray[3][0] != 'x')
    {
        ERROR("ERROR (AND): invalid operand\n");
        exit(4);
    }

    // Get DR
    DR = regnum(&tokArray[1][1]);

    // Get SR1
    SR1 = regnum(&tokArray[2][1]);

    // Get Operand2: SR2 or imm5
    if (tokArray[3][0] == 'r') // SR2
    {
        SR2 = regnum(&tokArray[3][1]);

        return ((5 << 12) | (DR << 9) | (SR1 << 6) | (SR2));
    }
    else // Imm5
    {
        if (tokArray[3][0] == '#')
        {
            imm5 = my_atoi(&tokArray[3][1]);
        }

        if (tokArray[3][0] == 'x')
        {
            imm5 = hex2i(&tokArray[3][1]);
        }

        if (imm5 > 15 || imm5 < -16)
        {
            printf("ERROR (AND): Invalid Constant %d\n", imm5);
            exit(3);
        }

        imm5 = imm5 & 0x1F;

        return ((5 << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | (imm5));
    }
}

unsigned short br(const unsigned short lineAddress)
{
    int nzp = 0;
    unsigned int labelAddress = 0;
    int pcOffset9 = 0;

    if (!tokArray[1])
    {
        ERROR("ERROR (BR): Missing Operands\n");
        exit(4);
    }

    if (tokArray[2] || tokArray[3])
    {
        ERROR("ERROR (BR): Too many operands\n");
        exit(4);
    }

    // Get nzp
    if (!strcmp(tokArray[0], "brn"))
    {
        nzp = 4;
    }
    if (!strcmp(tokArray[0], "brz"))
    {
        nzp = 2;
    }
    if (!strcmp(tokArray[0], "brp"))
    {
        nzp = 1;
    }
    if (!strcmp(tokArray[0], "brzp"))
    {
        nzp = 3;
    }
    if (!strcmp(tokArray[0], "brnp"))
    {
        nzp = 5;
    }
    if (!strcmp(tokArray[0], "brnz"))
    {
        nzp = 6;
    }
    if (!strcmp(tokArray[0], "br") || !strcmp(tokArray[0], "brnzp"))
    {
        nzp = 7;
    }

    // Get label's address
    for (int i = 0; i < MAX_SYMBOLS; ++i)
    {
        if (!strcmp(tokArray[1], symbolTable[i].label))
        {
            labelAddress = symbolTable[i].address;
        }
    }

    if (labelAddress == 0)
    {
        ERROR("ERROR (BR): Undefined label\n");
        exit(1);
    }

    // lineAddress + 2 points to the next instruction
    // pcOffset9 specifies the number of instructions, forward
    // or backwards, to branch over
    pcOffset9 = (signed)(labelAddress - (lineAddress + 2)) / 2;

    if (pcOffset9 > 255 || pcOffset9 < -256)
    {
        ERROR("ERROR (BR): Invalid Label offset\n");
        exit(4);
    }

    pcOffset9 = pcOffset9 & 0x1FF;
    return ((nzp << 9) | (pcOffset9));
}

unsigned short jmp_ret(void)
{
    int baseR = 0;

    // 1. jmp
    if (!strcmp(tokArray[0], "jmp"))
    {
        if (!tokArray[1])
        {
            ERROR("ERROR: miss operand\n");
            exit(4);
        }

        if (tokArray[2] || tokArray[3])
        {
            ERROR("ERROR: Too many operand\n");
            exit(4);
        }

        if (tokArray[1][0] != 'r')
        {
            ERROR("ERROR: invalid operands\n");
            exit(4);
        }

        int i = 1;
        while (tokArray[1][i])
        {
            baseR = baseR * 10 + (tokArray[1][i] - '0');
            ++i;
        }

        return ((12 << 12) | (baseR << 6));
    }

    // 2. ret
    if (!strcmp(tokArray[0], "ret"))
    {
        if (tokArray[1] || tokArray[2] || tokArray[3])
        {
            ERROR("ERROR: too many operand\n");
            exit(4);
        }

        return ((12 << 12) | (7 << 6));
    }

    // Illegal opcode
    exit(2);
}

unsigned short jsr_jsrr(const unsigned short lineAddress)
{
    int pcOffset11 = 0;
    int baseR = 0;
    int labelAddress = 0;

    // 1. jsr
    if (!strcmp(tokArray[0], "jsr"))
    {
        if (!tokArray[1])
        {
            ERROR("ERROR (JSR): Missing Operands\n");
            exit(4);
        }

        if (tokArray[2] || tokArray[3])
        {
            ERROR("ERROR (JSR): Too many Operands\n");
            exit(4);
        }

        // Get label's address
        for (int i = 0; i < MAX_SYMBOLS; ++i)
        {
            if (!strcmp(tokArray[1], symbolTable[i].label))
            {
                labelAddress = symbolTable[i].address;
            }
        }

        if (!labelAddress)
        {
            ERROR("ERROR (JSR): Undefined Label\n");
            exit(1);
        }

        pcOffset11 = (labelAddress - (lineAddress + 2)) / 2;

        if (pcOffset11 > 1023 || pcOffset11 < -1024)
        {
            ERROR("ERROR (JSR): Invalid Label Offset\n");
            exit(3);
        }

        pcOffset11 = pcOffset11 & 0x7FF;

        return ((4 << 12) | (1 << 11) | (pcOffset11));
    }

    // 2. jsrr
    if (!strcmp(tokArray[0], "jsrr"))
    {
        if (!tokArray[1])
        {
            ERROR("ERROR (JSRR): Missing Operands\n");
            exit(4);
        }

        if (tokArray[2] || tokArray[3])
        {
            ERROR("ERROR (JSRR): Too many Operands\n");
            exit(4);
        }

        if (tokArray[1][0] != 'r')
        {
            ERROR("ERROR (JSRR): Invalid Operands\n");
            exit(4);
        }

        // Get baseR
        int i = 1;
        while(tokArray[1][i])
        {
            baseR = baseR * 10 + (tokArray[1][i] - '0');
            ++i;
        }

        return ((4 << 12) | (baseR << 6));
    }

    // Illegal Opcode
    exit(2);
}

unsigned short ldb(void)
{
    int DR = 0;
    int baseR = 0;
    int bOffset6 = 0;

    if (!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (LDB): Missing Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
    {
        ERROR("ERROR (LDB): Invalid Operand\n");
        exit(4);
    }
    if (tokArray[3][0] != '#' && tokArray[3][0] != 'x')
    {
        ERROR("ERROR (LDB): Invalid Operand\n");
        exit(4);
    }

    // Get DR
    int i = 1;
    while(tokArray[1][i])
    {
        DR = DR * 10 + (tokArray[1][i] - '0');
        ++i;
    }
    // Get baseR
    i = 1;
    while(tokArray[2][i])
    {
        baseR = baseR * 10 + (tokArray[2][i] - '0');
        ++i;
    }

    // Get bOffset6
    i = 0;
    while (tokArray[3][i] != '#' && tokArray[3][i] != 'x')
    {
        ++i;
    }
    if (tokArray[3][i] == '#')
    {
        while(tokArray[3][i + 1])
        {
            bOffset6 = bOffset6 * 10 + (tokArray[3][i + 1] - '0');
            ++i;
        }
    }

    if (tokArray[3][i] == 'x')
    {
        bOffset6 = hex2i(&tokArray[3][i + 1]);
    }

    // Error checking
    if (bOffset6 > 31 || bOffset6 < -32)
    {
        ERROR("ERROR (LDB): Invalid Offset\n");
        exit(3);
    }

    /*
     * bOffset6's size should be between 0 and 6
    */
    bOffset6 = bOffset6 & 0x3F;

    return ((2 << 12) | (DR << 9) | (baseR << 6) | (bOffset6));
}


unsigned short ldw(void)
{
    int DR = 0;
    int baseR = 0;
    int offset6 = 0;

    if (!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (LDW): Missing Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
    {
        ERROR("ERROR (LDW): Invalid Operand\n");
        exit(4);
    }

    if (tokArray[3][0] != '#' && tokArray[3][0] != 'x')
    {
        ERROR("ERROR (LDW): Invalid operand\n");
        exit(4);
    }

    // Get DR
   int i = 1;
    while(tokArray[1][i])
    {
        DR = DR * 10 + (tokArray[1][i] - '0');
        ++i;
    }

    // Get baseR
    i = 1;
    while(tokArray[2][i])
    {
        baseR = baseR * 10 + (tokArray[2][i] - '0');
        ++i;
    }

    // Get Offset6
    i = 0;
    while(tokArray[3][i] != '#' && tokArray[3][i] != 'x')
    {
        ++i;
    }
    if (tokArray[3][i] == '#')
    {
        offset6 = my_atoi(&tokArray[3][i + 1]);
    }

    if (tokArray[3][i] == 'x')
    {
        offset6 = hex2i(&tokArray[3][i + 1]);
    }

    // Error checking
    if (offset6 > 31 || offset6 < -32)
    {
        ERROR("ERROR (LDW): Invalid offset\n");
        exit(3);
    }

    offset6 = offset6 & 0x3F;

    return ((6 << 12) | (DR << 9) | (baseR << 6) | (offset6));
}

unsigned short lea(const unsigned short lineAddress)
{
    int DR = 0;
    int pcOffset9 = 0;
    int labelAddress = 0;

    if (!tokArray[1] || !tokArray[2])
    {
        ERROR("ERROR (LEA): Missing Operands\n");
        exit(4);
    }

    if (tokArray[3])
    {
        ERROR("ERROR (LEA): Too many operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r')
    {
        ERROR("ERROR (LEA): Invalid operands\n");
        exit(4);
    }

    // Get DR
    int i = 1;
    while(tokArray[1][i])
    {
        DR = DR * 10 + (tokArray[1][i] - '0');
        ++i;
    }

    // Get pcOffset9
    for (int i = 0; i < MAX_SYMBOLS; ++i)
    {
        if (!strcmp(tokArray[2], symbolTable[i].label))
        {
            labelAddress = symbolTable[i].address;
        }
    }

    if (labelAddress == 0)
    {
        ERROR("ERROR (LEA): Undefined label\n");
        exit(1);
    }

    pcOffset9 = (labelAddress - (lineAddress + 2)) / 2;

    if (pcOffset9 > 255 || pcOffset9 < -256)
    {
        ERROR("ERROR (LEA): Invalid offset\n");
        exit(4);
    }

    pcOffset9 = pcOffset9 & 0x1FF;

    return ((14 << 12) | (DR << 9) | (pcOffset9));
}

unsigned short rti(void)
{
    if (tokArray[1] || tokArray[2] || tokArray[3])
    {
        ERROR("ERROR (RTI): Too many operands\n");
        exit(4);
    }

    return ((8 << 12));
}

unsigned short shf(void)
{
    int DR = 0;
    int SR = 0;
    int amount4 = 0;

    if (!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (SHF): Missing Operands\n");
        exit(4);
    }

    if (tokArray[3][0] != '#' && tokArray[3][0] != 'x')
    {
        ERROR("ERROR (SHF): Invalid Operands\n");
        exit(4);
    }

    // Get DR
    int i = 1;
    while (tokArray[1][i])
    {
        DR = DR * 10 + (tokArray[1][1] - '0');
        ++i;
    }

    // Get SR
    i = 1;
    while (tokArray[2][i])
    {
        SR = SR * 10 + (tokArray[2][1] - '0');
        ++i;
    }

    // Get amount4
    i = 0;
    while (tokArray[3][i] != 'x' && tokArray[3][i] != '#')
    {
        ++i;
    }

    if (tokArray[3][i] == '#')
    {
        while (tokArray[3][i + 1])
        {
            amount4 = amount4 * 10 + (tokArray[3][i + 1] -'0');
            ++i;
        }
    }

    if (tokArray[3][i] == 'x')
    {
        amount4 = hex2i(&tokArray[3][i + 1]);
    }

    // Error checking
    if (amount4 > 7 || amount4 < 0)
    {
        ERROR("ERROR (SHF): Invalid constant\n");
        exit(3);
    }

    amount4 = amount4 & 0x0F;
    if (!strcmp(tokArray[0], "lshf"))
    {
        return ((13 << 12) | (DR << 9) | (SR << 6) | (amount4));
    }

    if (!strcmp(tokArray[0], "rshfl"))
    {
        return ((13 << 12) | (DR << 9) | (SR << 6) | (1 << 4) | (amount4));
    }

    if (!strcmp(tokArray[0], "rshfa"))
    {
        return ((13 << 12) | (DR << 9) | (SR << 6) | (3 << 4) | (amount4));
    }

    // Illegal Opcode
    exit(2);
}

unsigned short stb(void)
{
    int SR = 0;
    int BaseR = 0;
    int boffset6 = 0;

    if (!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (STB): Missing Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
    {
        ERROR("ERROR (STB): Invalid Operands\n");
        exit(4);
    }

    if (tokArray[3][0] != '#' && tokArray[3][0] != 'x')
    {
        ERROR("ERROR (STB): Invalid Operands\n");
        exit(4);
    }

    // Get SR
    SR = regnum(&tokArray[1][1]);

    // Get BaseR
    BaseR = regnum(&tokArray[2][1]);

    // Get boffset6
    int i = 0;
    if (tokArray[3][i] == '#')
    {
        boffset6 = my_atoi(&tokArray[3][i + 1]);
    }

    if (tokArray[3][i] == 'x')
    {
        boffset6 = hex2i(&tokArray[3][i + 1]);
    }

    if (boffset6 > 31 || boffset6 < -32)
    {
        ERROR("ERROR (STB): Invalid constant\n");
        exit(3);
    }

    boffset6 = boffset6 & 0x3F;

    return ((3 << 12) | (SR << 9) | (BaseR << 6) | (boffset6));
}

unsigned short stw(void)
{
    int SR = 0;
    int BaseR = 0;
    int offset6 = 0;

    if (!tokArray[1] || !tokArray[2] || !tokArray[3])
    {
        ERROR("ERROR (STW): Missing Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
    {
        ERROR("ERROR (STW): Invalid operands\n");
        exit(4);
    }

    if (tokArray[3][0] != '#' && tokArray[3][0] != 'x')
    {
        ERROR("ERROR (STW): Invalid operands\n");
        exit(4);
    }

    // Get SR
    SR = regnum(&tokArray[1][1]);

    // Get BaseR
    BaseR = regnum(&tokArray[2][1]);

    // Get offset6
    int i = 0;
    if (tokArray[3][i] == '#')
    {
        offset6 = my_atoi(&tokArray[3][i + 1]);
    }

    if (tokArray[3][i] == 'x')
    {
        offset6 = hex2i(&tokArray[3][i + 1]);
    }

    if (offset6 > 31 || offset6 < -32)
    {
        ERROR("ERROR (STW): Invalid constant\n");
        exit(3);
    }

    offset6 = offset6 & 0x3F;

    return ((7 << 12) | (SR << 9) | (BaseR << 6) | (offset6));
}

unsigned short trap_halt(void)
{
    int trapvect8 = 0;

    // trap
    if (!strcmp(tokArray[0], "trap"))
    {
        if (!tokArray[1])
        {
            ERROR("ERROR (TRAP): Missing Operands\n");
            exit(4);
        }

        if (tokArray[2] || tokArray[3])
        {
            ERROR("ERROR (TRAP): Too many operands\n");
            exit(4);
        }

        if (tokArray[1][0] != 'x')
        {
            ERROR("ERROR (TRAP): Invalid trapvect8\n");
            exit(4);
        }

        // Get trapvect8
        trapvect8 = hex2i(&tokArray[1][1]);

        if (trapvect8 > 63 || trapvect8 < 0)
        {
            ERROR("ERROR (TRAP): Invalid trapvect8\n");
            exit(3);
        }

        return ((15 << 12) | (trapvect8));
    }

    // halt
    if (!strcmp(tokArray[0],"halt"))
    {
        if (tokArray[1] || tokArray[2] || tokArray[3])
        {
            ERROR("ERROR (HALT): Too many operands\n");
            exit(4);
        }

        return ((15 << 12) | (37));
    }

    // Illegal Opcode
    exit(2);
}

unsigned short not_xor(void)
{
    int DR = 0, SR = 0, SR1 = 0, SR2 = 0, imm5 = 0;

    // 1. not
    if (!strcmp(tokArray[0], "not"))
    {
        if (!tokArray[1] || !tokArray[2])
        {
            ERROR("ERROR (NOT): Missing Operand\n");
            exit(4);
        }

        if (tokArray[3])
        {
            ERROR("ERROR (NOT): Too Many Operands\n");
            exit(4);
        }

        if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
        {
            ERROR("ERROR (NOT): Invalid Register Operand\n");
            exit(4);
        }

        // Get DR
        DR = regnum(&tokArray[1][1]);

        // Get SR
        SR = regnum(&tokArray[2][1]);

        return ((9 << 12) | (DR << 9) | (SR << 6) | (63));
    }

    // 2. xor
    if (!strcmp(tokArray[0], "xor"))
    {
        if (!tokArray[1] || !tokArray[2] || !tokArray[3])
        {
            ERROR("ERROR (XOR): Missing Operand\n");
            exit(4);
        }

        if (tokArray[1][0] != 'r' || tokArray[2][0] != 'r')
        {

            ERROR("ERROR (XOR): Invalid Operand\n");
            exit(4);
        }

        if (tokArray[3][0] != 'r' && tokArray[3][0] != '#' && tokArray[3][0] != 'x')
        {
            ERROR("ERROR (XOR): invalid operand\n");
            exit(4);
        }

        // Get DR
        DR = regnum(&tokArray[1][1]);

        // Get SR1
        SR1 = regnum(&tokArray[2][1]);

        // Get Operand2
        // 1. SR2
        if (tokArray[3][0] == 'r')
        {
            SR2 = regnum(&tokArray[3][1]);

            return ((9 << 12) | (DR << 9) | (SR1 << 6) | (SR2));
        }
        // 2. imm5
        else
        {
            if (tokArray[3][0] == '#')
            {
                imm5 = my_atoi(&tokArray[3][1]);
            }

            if (tokArray[3][0] == 'x')
            {
                imm5 = hex2i(&tokArray[3][1]);
            }

            if (imm5 > 15 || imm5 < -16)
            {
                ERROR("ERROR (XOR): Invalid Constant\n");
                exit(3);
            }

            imm5 = imm5 & 0x1F;

            return ((9 << 12) | (DR << 9) | (SR1 << 6) | (1 << 5) | (imm5));
        }

    }

    // Illegal Opcode
    exit(2);
}

unsigned short orig(void)
{
    unsigned short baseAddress = 0;

    if (!tokArray[1])
    {
        ERROR("ERROR (.ORIG): Missing Operaqnd\n");
        exit(4);
    }

    if (tokArray[2] || tokArray[3])
    {
        ERROR("ERROR (.ORIG): Too many Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'x' && tokArray[1][0] != '#')
    {
        ERROR("ERROR (.ORIG): Invalid Operand\n");
        exit(4);
    }

    int i = 0;
    // Get baseAddress
    if (tokArray[1][i] == '#')
    {
        while(tokArray[1][i + 1])
        {
            if (tokArray[1][i + 1] >= '0' && tokArray[1][i + 1] <= '9')
            {
                baseAddress = baseAddress * 10 + (tokArray[1][i + 1] - '0');
                ++i;
            }
        }
    }

    i = 0;
    if (tokArray[1][i] == 'x')
    {
        baseAddress = hex2i(&tokArray[1][i + 1]);
    }

    // Error checking
    if (baseAddress > 65535 || baseAddress < 0)
    {
        ERROR("ERROR (.ORIG): Invalid Base Address\n");
        exit(3);
    }

    return baseAddress;
}

unsigned short fill(void)
{
    int value = 0;

    if (!tokArray[1])
    {
        ERROR("ERROR (.FILL): Missing Operand\n");
        exit(4);
    }

    if (tokArray[2] || tokArray[3])
    {
        ERROR("ERROR (.FILL): Too many Operands\n");
        exit(4);
    }

    if (tokArray[1][0] != 'x' && tokArray[1][0] != '#')
    {
        ERROR("ERROR (.FILL): Invalid Operand\n");
        exit(4);
    }

    // Get value
    int i = 0;
    if (tokArray[1][i] == '#')
    {
        value = my_atoi(&(tokArray[1][i + 1]));
    }

    i = 0;
    if (tokArray[1][i] == 'x')
    {
        value = hex2i(&tokArray[1][i + 1]);
    }

    if (value > 32767 && value < -32768)
    {
        ERROR("ERROR (.FILL): Invalid constant\n");
        exit(3);
    }

    return value;
}

unsigned short nop(void)
{
    if (tokArray[1] || tokArray[2] || tokArray[3])
    {
        ERROR("ERROR (NOP): Too many Operands\n");
        exit(4);
    }

    return (0);
}

int main(int argc, char* argv[])
{
    // Check parameters
    int num = 0;
    char* progName = NULL;
    char* asmName = NULL;
    char* objName = NULL;

    num = argc;
    // Parameter Error Checking
    if (num < 3)
    {
        ERROR("Usage: ./assemble <source.asm> <out.obj>\n");
        exit(4);
    }

    progName = argv[0];
    asmName = argv[1];
    objName = argv[2];

    printf("Program Name is '%s'\n", progName);
    printf("LC3b Assembly File is '%s'\n", asmName);
    printf("Output File is '%s'\n", objName);
    fflush(stdout);

    // FILE I/O
    inFile = fopen(asmName, "r");
    oFile = fopen(objName, "w");

    if (!inFile)
    {
        printf("Cannot open file '%s'\n", asmName);
        fflush(stdout);
        exit(4);
    }

    if (!oFile)
    {
        printf("Cannot open file '%s'\n", objName);
        fflush(stdout);
        exit(4);
    }

    int labelIndex = 0;
    int lineNum = 0;

    // First Pass
    while(1)
    {
        // Parse a line from an asm file
        parse();
        // To calculate the label's address
        ++lineNum;

        // Get whether the first token in a line from an asm file is a Label
        int tokVal = tokToValue(tokArray[0]);

        // To fill the symbol table, need the label's address, so must process the baseAddress
        if (tokVal == -2) // -2 means the token is .orig
        {
            BaseAddress();
            lineNum = 1;
        }

        // The directive .orig must be the first, but not a label be a first
        // 考虑一种情况：如果不是现有.orig声明一个baseAddress，而是先出现一个label，这个label我该如何处理？毕竟我现在不知道这个label的address，所以要杜绝这种情况
        if (!foundOrig)
        {
            ERROR("'.orig' must be the first in an assembly source file\n");
            exit(4);
        }

        // Process label to fill the symbol table
        if (tokVal == -1) // -1 means the token is a Label
        {
            // Label Error checking
            // Invalid label
            int tokLength = strlen(tokArray[0]);

            if (!(tokLength >= 1 && tokLength <= 20))
            {
                ERROR("Invalid label! Label consist of 1 to 20 alphanumeric characters\n");
                exit(4);
            }

            if (tokArray[0][0] < 'a' || tokArray[0][0] > 'z')
            {
                ERROR("Invalid label! Label should start with a letter of the alphabet\n");
                exit(4);
            }

            if (!strcmp(tokArray[0], "add") || !strcmp(tokArray[0], "and") || !strcmp(tokArray[0], "brn") || !strcmp(tokArray[0], "brz") || !strcmp(tokArray[0], "brp") || !strcmp(tokArray[0], "br")
                    || !strcmp(tokArray[0], "brzp") || !strcmp(tokArray[0], "brnp") || !strcmp(tokArray[0], "brnz") || !strcmp(tokArray[0], "brnzp") || !strcmp(tokArray[0], "jmp")
                    || !strcmp(tokArray[0], "ret") || !strcmp(tokArray[0], "jsr") || !strcmp(tokArray[0], "jsrr") || !strcmp(tokArray[0], "ldb") || !strcmp(tokArray[0], "ldw")
                    || !strcmp(tokArray[0], "lea") || !strcmp(tokArray[0], "not") || !strcmp(tokArray[0], "ret") || !strcmp(tokArray[0], "rti") || !strcmp(tokArray[0], "lshf")
                    || !strcmp(tokArray[0], "rshfl") || !strcmp(tokArray[0], "rshfa") || !strcmp(tokArray[0], "stb") || !strcmp(tokArray[0], "stw") || !strcmp(tokArray[0], "trap")
                    || !strcmp(tokArray[0], "xor") || !strcmp(tokArray[0], "xor") || !strcmp(tokArray[0], "orig") || !strcmp(tokArray[0], "fill") || !strcmp(tokArray[0], "end"))
            {
                ERROR("Label cannot be the same as an opcode or directive\n");
                exit(4);
            }

            if (tokArray[0][0] == 'x')
            {
                ERROR("A valid label must start with a letter othern than 'x'\n");
                exit(4);
            }

            for (int i = 0; i < tokLength; ++i)
            {
                if ((tokArray[0][i] < 'a' && tokArray[0][i] > 'z') || (tokArray[0][i] < '0' && tokArray[0][i] > '9'))
                {
                    ERROR("A valid label should consist solely of alphanumeric characters: 'a' to 'z', '0' to '9'\n");
                    exit(4);
                }
            }

            if (!strcmp(tokArray[0], "in") || !strcmp(tokArray[0], "out") || !strcmp(tokArray[0], "getc") || !strcmp(tokArray[0], "puts"))
            {
                ERROR("A valid label cannot be IN, OUT, GETC, or PUTS\n");
                exit(4);
            }

            // Have the same Label
            for (int i = 0; i < labelIndex; ++i)
            {
                if (strcmp(symbolTable[i].label, tokArray[0]) == 0)
                {
                    ERROR("Have the same label\n");
                    exit(4);
                }
            }

            // Fill the symbol table
            int address = baseAddress + (lineNum - 2) * 2;
            symbolTable[labelIndex].address = address;
            strcpy(symbolTable[labelIndex].label, tokArray[0]);
            ++labelIndex;

            // If there's a .end after the label
            if (tokToValue(tokArray[1]) == -3)
            {
                break;
            }
        }

        if (tokVal == -3) // -3 means .end
        {
            break;
        }
    }

    // Let the file pointer to the beginning of the file
    rewind(inFile);
    lineNum = 0;

    // Second Pass
    while(1)
    {
        // Second pass
        // 1. Convert instruction into machine code
        // 2. write the machine code into the output file
        parse();
        ++lineNum;

        unsigned short lineAddress = baseAddress + (lineNum - 2) * 2;

        int tokValue = tokToValue(tokArray[0]);

        if (tokValue == -1) // first token is a label
        {
            // Delete label from tokenArray
            for (int i = 1; i < 6; ++i)
            {
                tokArray[i - 1] = tokArray[i];
            }
            tokArray[5] = '\0';

            tokValue = tokToValue(tokArray[0]);
            if (tokValue == -1)
            {
                ERROR("Invalid opcode\n");
                exit(2);
            }
        }

        // opcode arg1, arg2, arg3
       if (tokArray[4])
       {
           ERROR("Too many arguments\n");
           exit(4);
       }

       // Check register operand, but not Imm operand, cause Imm operand's check should based on the instruction itself.
       for (int i = 1; i <= 3; ++i)
       {
           if (tokArray[i])
           {
               int regNum = 0;
               if (tokArray[i][0] == 'r' && isdigit(tokArray[i][1]))
               {
                   int j = 1;
                   while(tokArray[i][j])
                   {
                       regNum = regNum * 10 + (tokArray[i][j] - '0');
                       ++j;
                   }

                   if (regNum < 0 || regNum > 7)
                   {
                       ERROR("Invalid register operand\n");
                       exit(4);
                   }
               }
           }
       }

       if (tokValue == -3) // .end directive
           break;

       unsigned short instruction = convert(tokValue, lineAddress);

       // 2. Write machine code into the output file
       fprintf(oFile, "0x%.4X\n", instruction);
    }

    // close file
    if (fclose(inFile) == EOF)
    {
        ERROR("Failed to close the assembly source file\n");
        exit(4);
    }

    if (fclose(oFile) == EOF)
    {
        ERROR("Failed to close the object file\n");
        exit(4);
    }

    return 0;
}
