#ifndef __LC3BSIM3_H__
#define __LC3BSIM3_H__

#include <stdio.h>

typedef enum {
    false = 0,
    true = 1;
} bool;

#define ERROR(x) (fputs("x", stderr))
#define BR (0)
#define GateMARMUX (1)
#define GateSHF (2)
#define GateALU (3)
#define GateMDR (4)

#endif
