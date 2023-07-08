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

// Following for I/E
#define GateTEMP (5)
#define GatePSR (6)
#define GatePCM2 (7)
#define GateSPMUX (8)
#define GateSSP (9)
#define GateUSP (10)
#define GateVECTOR (11)

#define VECTOREBASEADDRESS (0x0200)
#define TIMEOUT (299)
#define INTERRUPT_VECTOR (0x01)
#define ENABLE_INTERRUPT (1)
#define DISENABLE_INTERRUPT (0)

// Bus data
#define NODATA (0)

// ALUK
#define ADD (0)
#define AND (1)
#define XOR (2)
#define PASSA (3)

#endif
