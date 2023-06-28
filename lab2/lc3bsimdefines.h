#ifndef __LC3BSIMDEFINES__H
#define __LC3BSIMDEFINES__H

#define ADD 1
#define AND 5
#define BR 0
#define JMP_RET 12
#define JSR_R 4
#define LDB 2
#define LDW 6
#define LEA 14
#define RTI 8
#define SHF 13
#define STB 3
#define STW 7
#define TRAP 15
#define XOR 9

typedef enum {
    false = 0,
    true = 1
} bool;

#endif
