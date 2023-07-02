#ifndef __LC3BSIM3_H__
#define __LC3BSIM3_H__

#include <stdio.h>

typedef enum {
    false = 0,
    true = 1;
} bool;

#define ERROR(x) (fputs("x", stderr))
#define BR (0)

// GateID for driving bus
#define NONEGate (0)
#define GateMARMUX (1)
#define GateSHF (2)
#define GateALU (3)
#define GateMDR (4)

// Bus data
#define NODATA (0)

// ALUK
#define ADD (0)
#define AND (1)
#define XOR (2)
#define PASSA (3)

#endif
