/*
    Name : Zhang Runmin
    TyustID : 201920010125
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Cycle Level Simulator                                           */
/*                                                             */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lc3bsim3defines.h"

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();


/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18
#define GetInstructionField(x,y) (((instruction) >> (y)) & (((1) << (x - y + 1)) - (1)))
#define FROMADDER (GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION))
#define FROMBASER (GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION))

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) + (x[J3] << 3) + (x[J2] << 2) + (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;    /* run bit */
int BUS;        /* value of the bus */

typedef struct System_Latches_Struct{

    int PC,     /* program counter */
        MDR,    /* memory data register */
        MAR,    /* memory address register */
        IR,     /* instruction register */
        N,      /* n condition bit */
        Z,      /* z condition bit */
        P,      /* p condition bit */
        BEN;    /* ben register */

    int READY;  /* ready bit */
                /* The ready bit is also latched as you dont want the memory system to assert it at a bad point in the cycle*/

    int REGS[LC_3b_REGS]; /* register file. */

    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

    int STATE_NUMBER; /* Current State Number - Provided for debugging */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  eval_micro_sequencer();
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == 0) {
        ERROR("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = 0;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == 0) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = 0;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
        go();
        break;

    case 'M':
    case 'm':
        scanf("%i %i", &start, &stop);
        mdump(dumpsim_file, start, stop);
        break;

    case '?':
        help();
        break;
    case 'Q':
    case 'q':
        printf("Bye.\n");
        exit(0);

    case 'R':
    case 'r':
        if (buffer[1] == 'd' || buffer[1] == 'D')
            rdump(dumpsim_file);
        else {
            scanf("%d", &cycles);
            run(cycles);
        }
        break;

    default:
        ERROR("Invalid Command\n");
        break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n", i, ucode_filename);
            exit(-1);
        }

        /* Put in bits one at a time. */
        index = 0;

        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n", ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n", ucode_filename, i, j);
                exit(-1);
            }

            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
            index++;
        }

        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n", ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        ERROR("Error: Program file is empty\n");
        exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
            program_filename, ii);
            exit(-1);
        }

        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
        ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n", argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        ERROR("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here                                        */
/***************************************************************/

// Get current cycle's control bit to determine next state's address
void getCurrentCycleControlBits(int &J, int &COND, int &IRD)
{
    *J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    *COND = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    * IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
}

// Get current cycle's instruction to determine next state's address
unsigned int getCurrentCycleInstruction()
{
    int current_pc = CURRENT_LATCHES.PC;
    int baseAddress = current_pc >> 1;

    int lowInstruction = MEMORY[baseAddress][0];
    int highInstruction = MEMORY[baseAddress][1];

    return ((highInstruction << 8) | (lowInstruction));
}

unsigned int instruction;
unsigned int nextStateAddress;
void eval_micro_sequencer() {

  /*
   * Evaluate the address of the next state according to the
   * micro sequencer logic. Latch the next microinstruction.
   */

    // 1. What's going on in the current clock cycle
    int J, COND, IRD;
    getCurrentCycleControlBits(&J, &COND, & IRD);

    // 2. The LC-3b instruction that is being executed
    instruction = GetCurrentCycleInstruction();
    unsigned int opcode = GetInstructionField(&instruction,15,12);
    unsigned int jsrOrJsrr = GetInstructionField(&instruction,11,11);
    unsigned int n = GetInstructionField(11,11);
    unsigned int z = GetInstructionField(10,10);
    unsigned int p = GetInstructionField(9,9);

    // 3. If the LC-3b isntruction is a BR, whether the conditions for the branch have been met
    //      (i.e, the state of the relevant condion codes
    int BEN = CURRENT_LATCHES.BEN;

    // 4. If a memory operatin is in progress, whether it is completing during this cycle
    int R = CURRENT_LATCHES.READY;

    // Evaluate next state's address
    if (IRD == 0)
    {
        if (COND == 2) // BEN
        {
            nextStateAddress = J | (BEN << 2);
        }
        else if (COND = 1) // R
        {
            nextStateAddress = J | (R << 1);
        }
        else if (COND = 3) // IR[11], JSR_R
        {
            nextStateAddress = J | (jsrOrJsrr);
        }
        else // Original address
        {
            nextStateAddress = J;
        }
    }
    else // Decode
    {
        nextStateAddress = opcode;
        NEXT_STATE .BEN = ((n && CURRENT_LATCHES.N) || (z && CURRENT_LATCHES.Z) || (p && CURRENT_LATCHES.P));
    }

    NEXT_LATCHES.STATE_NUMBER = nextStateAddress;

    // Latch next state's microinstruction
    for (int i = 0;i <CONTROL_STORE_BITS; ++i)
    {
        NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
    }
}

// Analyse whether there's a memory access request
bool access_memory()
{
    return GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool write_memory()
{
    return GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool stw()
{
    return GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
}


unsigned remainingCycles = MEM_CYCLES;
void cycle_memory() {
  /*
   * This function emulates memory and the WE logic.
   * Keep track of which cycle of MEMEN we are dealing with.
   * If fourth, we need to latch Ready bit at the end of
   * cycle to prepare microsequencer for the fifth cycle.
   */
    if (!access_memory())
    {
        return ;
    }

    remainingCycles = (remainningCycles  == 1) ? MEM_CYCLES : (remainingCycles - 1);

    if (remainingCycles == 1)
    {
        NEXT_LATCHES.READY = 1;
    }
    else
    {
        NEXT_LATCHES.READY = 0;
    }

    if (CURRENT_LATCHES.READY)
    {
        int address = CURRENT_LATCHES.MAR;
        int baseAddress = address >> 1;
        int lsb = address & 0x1;

        int data = CURRENT_LATCHES.MDR;
        int lowData = data & 0xFF;
        int highData = (data >> 8) & 0xFF;

        if (write_memroy())
        {
            if (stw())
            {
                MEMROY[baseAddress][0] = lowData;
                MEMORY[baseAddress][1] = highData;
            }
            else
            {
                MEMORY[baseAddress][lsb] = lowData;
            }
        }
        else
        {
            NEXT_LATCHES.MDR = ((MEMORY[baseAddress][1] << 8) & 0xFF) | (MEMORY[baseAddress][0]);
        }

        NEXT_LATCHES.READY = 0;
    }
}


int gateID;
void eval_bus_drivers() {

  /*
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers
   *            Gate_MARMUX,
   *            Gate_PC,
   *            Gate_ALU,
   *            Gate_SHF,
   *            Gate_MDR.
   */
    gateID = NONEGate;

    if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCION))
    {
        gateID = GateMARMUX;
    }

    if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GatePC;
    }

    if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GateSHF;
    }

    if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GatALU;
    }

    if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = Gate_MDR;
    }
}

int adder()
{
    int op1, op2;
    // Get op1
    int op = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTIOON);
    if (op == 3)
    {
        op1 = signExt(GetInstructinField(10,0));
    }
    else if (op == 2)
    {
        op1 = signExt(GetInstructionField(8,0));
    }
    else if (op == 1)
    {
        op1 = signExt(GetInstructionField(5,0));
    }
    else
    {
        op1 = 0;
    }

    if (GetLSHF(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        op1 = op1 << 1;
    }

    // Get op2
    if(FROMBASER)
    {
        // SR1MUX
        if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION))
        {
            op2 = CURRENT_LATCHES.REGS[GetInstructionField(8,6)];
        }
        else
        {
            op2 = CURRENT_LATCHES.REGS[GetInstructionField(11,9)];
        }
    }
    else
    {
        op2 = CURRENT_LATCHES.PC;
    }


    return op1 + op2;
}

int trap()
{
    return (GetInstructionField(7,0)) << 1;
}

int MARMUX()
{
    if (FROMADDER)
    {
        return adder();
    }
    else
    {
        return trap();
    }
}

int SR1MUX()
{
    if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)))
    {
        return CURRENT_LATCHES.REGS[GetInstructionField(8,6)];
    }
    else
    {
        return CURRENT_LATCHES.REGS[GetInstructionField(11,9)];
    }
}

int SR2MUX()
{
    if (GetInstructionField(5,5)) // imm5
    {
        return GetInstructionField(4,0);
    }
    else // register rs2
    {
        return CURRENT_LATCHES.REGS[GetInstructionField(2,0)];
    }
}

int ALU_Result()
{
    int op1, op2;
    op1 = SR1MUX();
    op2 = SR2MUX();

    int ALUK = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
    if (ALUK == ADD)
    {
        return op1 + op2;
    }

    if (ALUK == AND)
    {
        return op1 & op2;
    }

    if (ALUK == XOR)
    {
            return op1 ^ op2;
    }

    if (ALUK == PASSA)
    {
        return op1;
    }
}

int bus_data;
void drive_bus() {

  /*
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers.
   */

    if (gateID == NONEGate)
    {
        bus_data = NODATA;
    }

    if (gateID == GateMARMUX)
    {
        bus_data = Low16bits(MARMUX());
    }

    if (gateID == GatePC)
    {
        bus_data = Low16bits(CURRENT_LATCHES.PC);
    }

    if (gateID == GateSHF)
    {
        int oprand = SR1MUX();

        int amount4 = GetInstructionField(3,0);
        if (GetInstructionField(4,4)) // Right shift
        {
            if (GetInstructionField(5,5)) // Arithmetic
            {
                bus_data = Low16bits(signExt(operand, 16) >> amount);
            }
            else
            {
                bus_data = Low16bits(operand >> amount4);
            }
        }
        else // Lefth shift
        {
            bus_data = Low16bits(operand << amount4);
        }
    }

    if (gateID == GateALU)
    {
        bus_data = Low16bits(ALU_Result());
    }

    if (gateID == GateMDR)
    {
        if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) // LDW
            return Low16bits(CURRENT_LATCHES.MDR);
        else // LDB
            return Low16bits(signExt(CURRENT_LATCHES.MDR & 0xFF, 8));
    }
}

bool ldw()
{
    return GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
}

void latch_datapath_values() {

  /*
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */

    // There are 7 LD.xx that we should latch
    // 1. LD.MAR
    if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        NEXT_LATCHES.MAR = Low16bits(bus_data);
    }
    // 2. LD.MDR
    if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        // 2.1 From memory
        if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION))
        {
            if (CURRENT_LATCHES.READY)
            {
                int address = CURRENT_LATCHES.MAR;
                int baseAddress = address >> 1;
                int lsb = address & 0x1;

                // Read (always read, if GateMDR is zero, read will not have side-effective)
                if (GetR_W(CURRENT_LATCHES.MICROINSTRUCTION) == 0)
                {
                    int lowData = MEMORY[baseAddress][0] & 0xFF;
                    int highData = MEMORY[baseAddress][1] & 0xFF;
                    NEXT_LATCHES.MDR = ((highData << 8) | lowData);
                }
            }
        }

        // 2.2 From BUS
        if (ldw())
        {
            NEXT_LATCHES.MDR = Low16Bits(bus_data);
        }
        else
        {
            NEXT_LATCHES.MDR = bus_data & 0xFF;
        }
    }

    // 3. LD.IR
    if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        NEXT_LATCHES.IR = Low16bits(bus_data);
    }

    // 4. LD.BEN
    // 5. LD.REG
    // 6. LD.CC
    // 7. LD.PC
}
