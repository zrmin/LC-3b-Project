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
#include "lc3bsim4defines.h"
#include "utils.h"

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
    COND2, COND1, COND0,
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
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,

    // Add following control bits
    SPMUX,
    EXCMUX,
    IEMUX,
    LD_TEMP,
    LD_PSR,
    LD_PRIV,
    LD_SavedSSP,
    LD_SavedUSP,
    LD_EX,
    LD_EXCV,
    LD_Vector,
    GATE_TEMP,
    GATE_PSR,
    GATE_PCM2,
    GATE_SPMUX,
    GATE_SSP,
    GATE_USP,
    GATE_Vector,
    ALIGN,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((X[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }
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
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + (x[DRMUX0])); }
int GetSR1MUX(int *x)        { return((X[sr1mux1] << 1) + (x[SR1MUX0])); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }

// Add functions to get the control bits.
int GetSPMUX(int *x)        { return(x[SPMUX]); }
int GetEXCMUX(int *x)       { return(x[EXCMUX]); }
int GetIEMUX(int *x)        { return(x[IEMUX]); }
int GetLD_TEMP(int *x)      { return(x[LD_TEMP]); }
int GetLD_PSR(int *x)       { return(x[LD_PSR]); }
int GetLD_PRIV(int *x)      { return(x[LD_PRIV]); }
int GetLD_SavedSSP(int *x)   { return(x[LD_SavedSSP]); }
int GetLD_SavedUSP(int *x)    { return(x[LD_SavedUSP]); }
int GetLD_EX(int *x)        { return(x[LD_EX]); }
int GetLD_EXCV(int *x)      { return(x[LD_EXCV]); }
int GetLD_Vector(int *x)    { return(x[LD_Vector]); }
int GetALIGN(int *x)        { return(x[ALIGN]); }
int GetGATE_TEMP(int *x)    { return(x[GATE_TEMP]); }
int GetGATE_PSR(int *x)     { return(x[GATE_PSR]);  }
int GetGATE_PCM2(int *x)    { return(x[GATE_PCM2]); }
int GetGATE_SPMUX(int *x)   { return(x[GATE_SPMUX]);    }
int GetGATE_SSP(int *x)     { return(x[GATE_SSP]);  }
int GetGATE_USP(int *x)     { return(x[GATE_USP]);  }
int GetGATE_VECTOR(int *x)  { return(x[GATE_VECTOR]);   }


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

    // Adding some regs to support interrupt and exception
    int INTV; /* Interrupti Vector Register */
    int EXCV; /* Exception Vector Register */
    int SSP: /* Supervisor Stack Pointer */
    int USP; /* User Stack Pointer */
    int PSR; /* Processor Status Register */
    int TEMP; /* Temporary Register to store PC before PC is incremented */
    int VECTOR; /* Store Interrupt/Exception Handler address */
    int EX; /* EXception Register */
    int INT; /* Interrupt Register */
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

  // Timer interrupt
  if (CYCLE_COUNT == 299)
  {
      NEXT_LATCHES.INT = 1;
      NEXT_LATCHES.INTV = 0x01;
  }
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

    RUN_BIT = true;
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
void getCurrentCycleControlBits(int *J, int *COND, int *IRD)
{
    *J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    *COND = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    * IRD = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
}

unsigned int instruction;
unsigned int nextStateAddress;
void eval_micro_sequencer() {
printf("Begining Caculate next state's address\n");
  /*
   * Evaluate the address of the next state according to the
   * micro sequencer logic. Latch the next microinstruction.
   */

    // 1. What's going on in the current clock cycle
    int J, COND, IRD;
    getCurrentCycleControlBits(&J, &COND, & IRD);

    // 2. The LC-3b instruction that is being executed
    instruction = CURRENT_LATCHES.IR;
    unsigned int opcode = GetInstructionField(15,12);
    unsigned int jsrOrJsrr = GetInstructionField(11,11);
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
        else if (COND == 1) // R
        {
            nextStateAddress = J | (R << 1);
        }
        else if (COND == 3) // IR[11], JSR_R
        {
            nextStateAddress = J | (jsrOrJsrr);
        }
        else if (COND == 4)
        {
            if (CURRENT_LATCHES.INT)
            {
                nextStateAddress = J | (0X08);
                // TODO: If there's an interrupt in current state, we should shut down the global interrupt
                // If there's an interrupt in current state, then show down the interrupt in the next state.
                NEXT_LATCHES.INT = 0;
            }
        }
        else if (COND == 5)
        {
            nextStateAddress = J | (0X10);
        }
        else if (COND == 6)
        {
            if (CURRENT_LATCHES.EX & 1) // Illegal opcode exception
            {
                nextStateAddress = J | (0X10);
            }
        }
        else if (COND == 7)
        {
            if (CURRENT_LATCHES.EX & 1)
            {
                nextStateAddress = J |(0X20);
            }
        }
        else // Original address
        {
            nextStateAddress = J;
        }
    }
    else // Decode
    {
        nextStateAddress = opcode;
    }

    NEXT_LATCHES.STATE_NUMBER = nextStateAddress;

    // Latch next state's microinstruction
    for (int i = 0;i < CONTROL_STORE_BITS; ++i)
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

    remainingCycles = (remainingCycles  == 1) ? MEM_CYCLES : (remainingCycles - 1);

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

        if (write_memory())
        {
            if (stw())
            {
                MEMORY[baseAddress][0] = lowData;
                MEMORY[baseAddress][1] = highData;
            }
            else
            {
                MEMORY[baseAddress][lsb] = lowData;
            }
        }
        else
        {
            NEXT_LATCHES.MDR = ((MEMORY[baseAddress][1] << 8)) | (MEMORY[baseAddress][0]);
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
   *
   *            Gate_TEMP,
   *            Gate_PSR,
   *            Gate_PCM2,
   *            Gate_SPMUX,
   *            Gate_SSP,
   *            Gate_uSP,
   *            Gate_Vector
   */
    gateID = NONEGate;

    if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION))
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
        gateID = GateALU;
    }

    if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GateMDR;
    }

    // Add follwoing gate to support interrupts and exceptions
    if (GetGATE_TEMP(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GateTEMP;
    }

    if (GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GatePSR;
    }

    if (GetGATE_PCM2(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GatePCM2;
    }

    if (GetSGATE_SPMUX(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GateSPMUX;
    }

    if (GetGATE_SSP(CURRENT_LATCHES.MICROINSTRUTION))
    {
        gateID = GateSSP;
    }

    if (GetGATE_USP(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GateUSP;
    }

    if (GetGATE_VECTOR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        gateID = GateVECTOR;
    }
}

int DRMUX_UNIT()
{
    if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION == 0))
        return GetInstructionField(11,9);
    else if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION == 1))
        return 7;
    else
        return 6;
}

int SR1MUX_UNIT()
{
    if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        return CURRENT_LATCHES.REGS[GetInstructionField(8,6)];
    }
    else if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION) == 2)
    {
        return CURRENT_LATCHES.REGS[6];
    }
    else
    {
        return CURRENT_LATCHES.REGS[GetInstructionField(11,9)];
    }
}

int SR2MUX_UNIT()
{
    if (GetInstructionField(5,5)) // imm5
    {
        return signExt(GetInstructionField(4,0), 5);
    }
    else // register sr2
    {
        return CURRENT_LATCHES.REGS[GetInstructionField(2,0)];
    }
}

int ADDR1MUX_UNIT()
{
    if(FROMBASER)
    {
        // SR1MUX
        return SR1MUX_UNIT();
    }
    else
    {
        return CURRENT_LATCHES.PC;
    }
}

int ADDR2MUX_UNIT()
{
    int op = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    if (op == PCOFFSET11)
    {
        return signExt(GetInstructionField(10,0), 11);
    }
    else if (op == PCOFFSET9)
    {
        return signExt(GetInstructionField(8,0), 9);
    }
    else if (op == OFFSET6)
    {
        return signExt(GetInstructionField(5,0), 6);
    }
    else
    {
        return 0;
    }
}

int LSHF1_UNIT(int value)
{
    if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        return value << 1;
    }

    return value;
}

int ADDER_UNIT()
{
    int op1, op2;

    // Get op1
    op1 = LSHF1_UNIT(ADDR2MUX_UNIT());

    // Get op2
    op2 = ADDR1MUX_UNIT();

    return op1 + op2;
}

int trap()
{
    return (GetInstructionField(7,0)) << 1;
}


int MARMUX_UNIT()
{
    if (FROMADDER)
    {
        return ADDER_UNIT();
    }
    else
    {
        return trap();
    }
}


int ALU_UNIT()
{
    int op1, op2;
    op1 = SR1MUX_UNIT();
    op2 = SR2MUX_UNIT();

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


int SPMUX_UNIT()
{
    int R6Value = SR1MUX_UNIT();
    if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        return R6Value + 2;
    }
    else
    {
        return R6Value - 2;
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
        bus_data = Low16bits(MARMUX_UNIT());
    }

    if (gateID == GatePC)
    {
        bus_data = Low16bits(CURRENT_LATCHES.PC);
    }

    if (gateID == GateSHF)
    {
        int operand = SR1MUX_UNIT();

        int amount4 = GetInstructionField(3,0);
        if (GetInstructionField(4,4)) // Right shift
        {
            if (GetInstructionField(5,5)) // Arithmetic
            {
                bus_data = Low16bits(signExt(operand, 16) >> amount4);
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
        bus_data = Low16bits(ALU_UNIT());
    }

    if (gateID == GateMDR)
    {
        if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) // Is a Word
            bus_data = Low16bits(CURRENT_LATCHES.MDR);
        else // is a byte
        {
            int address = CURRENT_LATCHES.MAR;
            int lsb = address & 0x1;
            if (lsb == 0)
            {
                bus_data = Low16bits(signExt(CURRENT_LATCHES.MDR & 0XFF, 8));
            }
            else
            {
                bus_data = Low16bits(signExt((CURRENT_LATCHES.MDR >> 8) & 0XFF, 8));
            }
        }
    }

    if (gateID == GateTEMP)
    {
        bus_data = Low16bits(CURRENT_LATCHES.TEMP);
    }

    if (gateID == GatePSR)
    {
        bus_data = Low16bits(CURRENT_LATCHES.PSR);
    }

    if (gateID == GatePCM2)
    {
        bus_data = Low16bits(CURRENT_LATCHES.PC - 2);
    }

    if (gateID == GateSPMUX)
    {
        bus_data = Low16bits(SPMUX_UNIT());
    }

    if (gateID == GateSSP)
    {
        bus_data = Low16bits(CURRENT_LATCHES.SSP);
    }

    if (gateID == GateUSP)
    {
        bus_data = Low16bits(CURRENT_LATCHES.USP);
    }

    if (gateID == GateVECTOR)
    {
        bus_data = Low16bits(CURRENT_LATCHES.VECTOR);
    }
}

bool is_ldw()
{
    return GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_mar()
{
    return GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_mdr()
{
    return GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_ir()
{
    return GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_ben()
{
    return GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_reg()
{
    return GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_cc()
{
    return GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_pc()
{
    return GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_temp()
{
    return GetLD_TEMP(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_psr()
{
    return GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_priv()
{
    return GetLD_PRIV(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_saved_ssp()
{
    return GetLD_SavedSSP(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_saved_usp()
{
    return GetLD_SavedUSP(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_ex()
{
    return GetLD_EX(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_excv()
{
    return GetLD_EXCV(CURRENT_LATCHES.MICROINSTRUCTION);
}

bool is_ld_vector()
{
    return GetLD_Vector(CURRENT_LATCHES.MICROINSTRUCTION);
}

void setcc()
{
    if ((Low16bits(bus_data) >> 15) & 0x1)
    {
         NEXT_LATCHES.N = 1;
         NEXT_LATCHES.Z = 0;
         NEXT_LATCHES.P = 0;

         // Set PSR[2:0], and PSR[15] should be 1
         NEXT_LATCHES.PSR &= 0x8000;
         NEXT_LATCHES.PSR |= 0x0004;
    }
    else if (Low16bits(bus_data) == 0)
    {
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 1;
        NEXT_LATCHES.P = 0;

        // Set PSR[2:0], and PSR[15] should be 1
        NEXT_LATCHES.PSR &= 0x8000;
        NEXT_LATCHES.PSR |= 0x0002;
    }
    else
    {
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 1;

        // Set PSR[2:0], and PSR[15] should be 1
        NEXT_LATCHES.PSR &= 0x8000;
        NEXT_LATCHES.PSR |= 0x0001;
    }
}

int PCMUX_UNIT()
{
    if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == FROMBUS)
    {
        return bus_data;
    }
    else if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION) == FROMPCPLUS2)
    {
        return CURRENT_LATCHES.PC + 2;
    }
    else // From Adder
    {
        return ADDER_UNIT();
    }
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
    if (is_ld_mar())
    {
        NEXT_LATCHES.MAR = Low16bits(bus_data);
    }
    // 2. LD.MDR
    if (is_ld_mdr())
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
        else
        {
            if (is_ldw())
            {
                NEXT_LATCHES.MDR = Low16bits(bus_data);
            }
            else
            {

                NEXT_LATCHES.MDR = Low16bits(bus_data & 0xFF);
            }
        }
    }

    // 3. LD.IR
    if (is_ld_ir())
    {
        NEXT_LATCHES.IR = Low16bits(bus_data);
    }

    // 4. LD.BEN
    if (is_ld_ben())
    {
        int n = GetInstructionField(11,11);
        int z = GetInstructionField(10,10);
        int p = GetInstructionField(9,9);

        NEXT_LATCHES.BEN = (n & CURRENT_LATCHES.N) || (z & CURRENT_LATCHES.Z) || (p & CURRENT_LATCHES.P);
    }

    // 5. LD.REG
    if (is_ld_reg())
    {
        int regnum = DRMUX_UNIT();
        NEXT_LATCHES.REGS[regnum] = Low16bits(bus_data);
    }

    // 6. LD.CC
    if (is_ld_cc())
    {
        setcc();
    }

    // 7. LD.PC
    if (is_ld_pc())
    {
        NEXT_LATCHES.PC = Low16bits(PCMUX_UNIT());
    }

    // Following are added to support exceptions and interrupt
    // 1. LD.TEMP
    if (is_ld_temp())
    {
    }

    // 2. LD.PSR
    if (is_ld_psr())
    {
    }

    // 3. LD.PRIV
    if (is_ld_priv())
    {
    }

    // 4. LD.SavedSSP
    if (is_ld_savedssp())
    {
    }

    // 5. LD.SavedUSP
    if (is_ld_savedusp())
    {
    }

    // 6. LD.EX
    if (is_ld_ex())
    {
    }

    // 7. LD.EXCV
    if (is_ld_excv())
    {
    }

    // 8. LD.VECTOR
    if (is_ld_vector())
    {
    }
}
