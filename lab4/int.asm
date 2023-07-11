; Interrupt handler
; The interrupt handler you write should
; increment location (word-sized access) x4000 by 1.
.ORIG x1200

;push R0 and R1
ADD R6, R6, #-2 ; run 346:  R6 = x2FFA, P = 1
STW R0, R6, #0  ; run 364: M[x2FFA] = R0 = xC003
ADD R6, R6, #-2 ; run 373: R6 = x2FF8, P = 1
STW R1, R6, #0  ; run 390: M[x2FF8] = R1 = x005C

;add 1 to data at x4000
LEA R1, ADR     ; run 400: R1 = 0X121C
LDW R0, R1, #0  ; run 417: R0 = M[x121C] = x4000
LDW R1, R0, #0  ; run 434: R1 = M[x4000] = x0001, P = 1
ADD R1, R1, #1  ; run 444: R1 = x0002, P = 1
STW R1, R0, #0  ; run 461: M[x4000] = x0002

;pop R0 and R1
LDW R1, R6, #0  ; run 478: R1 = M[R6] = M[x2FF8] = x005C, P = 1
ADD R6, R6, #2  ; run 488: R6 = x2FFA, P = 1
LDW R0, R6, #0  ; run 505: R0 = M[x2FFA] = xC003, N = 1
ADD R6, R6, #2  ; run 515: R6 = x2FFC, P = 1

RTI             ; run 525: MAR = x2FFC
                ; run 530: MDR = OrignPC = x3016
                ; run 531: PC = x3016
                ; run 532: MAR = x2FFE, R6 = x2FF3
                ; run 537: MDR = OriginPSR = x8001
                ; run 538: R6 = x3000
                ; run 540: R6 = x0000
ADR .FILL x4000 ; lineAddress = x121C
.END
