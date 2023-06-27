
/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lc3bsimdefines.h"

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

typedef enum {
    false = 0,
    true = 1
} bool;

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)
#define GetInstructionBit(x) ((instruction) >> (x))
#define GetInstructionField(x,y) ((instruction) >> (y) & ((1) << (y - x + 1) - (1)))
/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
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
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
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
    printf("Error: Program file is empty\n");
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

// DECODE
int instruction;

int DR, SR1, SR2, imm5;
bool add_imm5;
void add()
{
    DR = GetInstructionField(11,9);
    SR1 = GetInstructionField(8.6);

    int choose = GetInstructionBit(5);
    if (choose)
    {i
        add_imm5 = true;
        imm5 = GetInstructinField(4,0);
    }
    else
    {
        add_imm5 = false;
        SR2 = GetInstructionField(2,0);
    }
}

bool and_imm5;
void and()
{
    DR = GetInstructionField(11,9);
    SR1 = GetInstructionField(8,6);

    int choose = GetInstructionBit(5);
    if (choose)
    {
        and_imm5 = true;
        imm5 = GetInstructionField(4,0);
    }
    else
    {
        and_imm5 = false;
        SR2 = GetInstructinField(2,0);
    }
}

int n, z, p, PCoffset9;
void br()
{
    n = GetInstructionBit(11);
    z = GetInstructionBit(10);
    p = GetInstructionBit(9);
    PCoffset9 = GetInstructionField(8,0);
}

int BaseR;
void jmp_ret()
{
    // If BaseR == 7, RET
    // Else JMP
    BaseR = GetInstructionField(8,6);
}

int PCoffset11;
void jsr_r()
{
    if (GetInstructionBit(11))
    {
        PCoffset11 = GetInstructionField(10,0);
    }
    else
    {
        BaseR = GetInstructionField(8,6);
    }
}

int boffset6;
void ldb()
{
    DR = GetInstructionField(11,9);
    BaseR = GetInstructionField(8,6);
    boffset6 = GetInstructionField(5,0);
}

int offset6;
void ldw()
{
    DR = GetInstructionField(11,9);
    BaseR = GetInstructionField(8,6);
    offset6 = GetInstructionField(5,0);
}

void lea()
{
    DR = GetInstructionField(11,9);
    PCoffset9 = GetInstructionField(8,0);
}

void rti()
{

}

int LSHF, RSHFL, RSHFA;
void shf()
{
    DR = GetInstructionField(11,9);
    SR = GetInstructionField(8,6);
    amount4 = GetInstructionField(3,0);

    int choose = GetInstructionField(5,4);
    if (choose == 0)
        LSHF = TRUE;
    else if (choose == 1)
        RSHFL = TRUE;
    else
        RSHFA = TRUE;
}

void stb()
{
    SR = GetInstructionField(11,9);
    BaseR = GetInstructionField(8,6);
    boffset6 = GetInstructionField(5,0);
}

void stw()
{
    SR = GetInstructionField(11,9);
    BaseR = GetInstructionField(8,6);
    offset6 = GetInstructionField(5,0);
}

int trapvect8;
void trap()
{
    trapvect8 = GetInstructionField(7,0);
}

int SR;
void xor()
{
    DR = GetInstructionField(11,9);
    if (GetInstructionBit(5))
    {
        SR = GetInstructionField(8,6);
        imm5 = GetInstructionField(4,0);
    }
    else
    {
        SR1 = GetInstructionField(8,6);
        SR2 = GetInstructionField(2,0);
    }
}

void decode(int opcode)
{
    switch (opcode)
    {
        case ADD:
            add();
            break;
        case AND:
            and();
            break;
        case BR:
            br();
            break;
        case JMP_RET:
            jmp_ret();
            break;
        case JSR_R:
            jsr_r();
            break;
        case LDB:
            ldb();
            break;
        case LDW:
            ldw();
            break;
        case LEA:
            lea();
            break;
        case RTI:
            rti();
            break;
        case SHF:
            shf();
            break;
        case STB:
            stb();
            break;
        case STW:
            stw();
            break;
        case TRAP:
            trap();
            break;
        case XOR:
            xor();
            break;
        default:
            printf("Error: Unknown Opcode\n");
            exit(2);
    }
}

// EXE
int TEMP_DR;
int TEMP_PC;
void add_exe()
{
    if (add_imm5)
    {
        TEMP_DR = SR1 + imm5;
    }
    else
    {
        TEMP_DR = SR1 + SR2;
    }
}

void and_exe()
{
    if (and_imm5)
    {
        TEMP_DR = SR1 & imm5;
    }
    else
    {
        TEMP_DR = SR1 & SR2;
    }
}

int signExtPCoffset9(const int PCoffset9)
{
    // Get sign bit
    int signBit = (PCoffset9 >> 8) & 0x1;

    // Sign Extension
    if (signBit)
    {
        return ((PCoffset9 & 0xFF) | 0xFFFFFF00);
    }

    return PCoffset9;
}

void br_exe()
{
    if (n && N || z && Z || p && P)
    {
        TEMP_PC = CURRENT_LATCHES.PC + 4 + (signExtPCoffset9(PCoffset9) << 1);
    }
}

void jmp_ret_exe()
{
    TEMP_PC = BaseR;
}

int signExtPCoffset11(const int PCoffset11)
{
    // Get sign bit
    int signBit = (PCoffset11 >> 10) & 0x1;

    // Sign Extension
    if (signBit)
    {
        return ((PCoffset11 & 0x3FF) | 0xFFFFFC00);
    }

    return PCoffset11;
}

void jsr_r_exe()
{
    if (is_jsrr)
    {
        TEMP_PC = BaseR;
    }
    else
    {
        TEMP_PC = CURRENT_LATCHES.PC + 4 + (signExtPCoffset11(PCoffset11) << 1);
    }
}

void ldb_exe()
{

}

void ldw_exe()
{
}

void lea_exe()
{
}

void rti_exe()
{
}

void shf_exe()
{
}

void stb_exe()
{
}

void stw_exe()
{
}

void trap_exe()
{
}

void xor_not_exe()
{
}

void execute(int opcode)
{
    switch (opcode)
    {
        case ADD:
            add_exe();
            break;
        case AND:
            and_exe();
            break;
        case BR:
            br_exe();
            break;
        case JMP_RET:
            jmp_ret_exe();
            break;
        case JSR_R:
            jsr_r_exe();
            break;
        case LDB:
            ldb_exe();
            break;
        case LDW:
            ldw_exe();
            break;
        case LEA:
            lea_exe();
            break;
        case RTI:
            rti_exe();
            break;
        case SHF:
            shf_exe();
            break;
        case STB:
            stb_exe();
            break;
        case STW:
            stw_exe();
            break;
        case TRAP:
            trap_exe();
            break;
        case XOR:
            xor_not_exe();
            break;
        default:
            printf("Error: Unknown Opcode!\n");
            exit(2);
    }
}

void process_instruction(){
  /*  function: process_instruction
   *
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */

    // Get one instruction form memory
    instruction = MEMORY[CURRENT_LATCHES.PC >> 1][0];

    // Decode this instruction
    int opcode = instruction >> 12;
    decode(opcode);

    // Execute
    execute(opcode);
}
