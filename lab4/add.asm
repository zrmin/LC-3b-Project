; The user program loaded into x3000. should do the following: initialize memory location
; x4000 to 1 and calculate the sum of the first 20 bytes stored in the memory locations beginning
; with xC000. This sum should first be stored at xC014. The next step would depend upon
; whether you are testing for protection exception or unaligned access exception. If you want to
; cause a protection exception,the user program should next store the sum at x0000. If you want
; to cause an unaligned access exception, the user program should store the sum at xC017.
; Finally, if you wish to test for an unknown opcode exception, you can simply use a .FILL psuedo
; op to create an instruction with an unimplemented opcode (1010 or 1011).

.ORIG x3000

;set M[x4000] to 1
   LEA R1, ADR1 ; run 10: R1 = 0x302a
   LDW R0, R1, #0   ; run 27: R0 = x4000,   P = 1
   AND R2, R2, #0   ; run 37: R2 = 0, Z = 1
   ADD R2, R2, #1   ; run 47: R2 = 1, P = 1
   STW R2, R0, #0   ; run 64: M[x4000] = 0x0001

;add first 20 bytes stored at M[xC000]
   LEA R1, ADR2     ; run 74: R1 = x302c
   LDW R0, R1, #0   ; run 91: R0 = xC000, Z = 1
   ADD R2, R2, #9   ; run 101: R2 = xa, P = 1
   ADD R2, R2, #10  ; run 111: R2 = x14 = 20D, P = 1
   AND R1, R1, #0   ; run 121: R1 = x0, Z = 1
LOOP LDB R3, R0, #0 ; run 138: R3 = x0012, P = 1; run 196: R3 = 0x0011, P = 1
                    ; run 254: R3 = x39, P = 1; run 312: R3 = x0023, P = 1, next state, a.k.a state
                                                ; 18, will detect a timer interrupt
                                                ; in state 18, next state is 42
                                                ; run 315: MDR = x8001
                                                ; run 316: R6 = x3000
                                                ; run 317: MAR = x2FFE, R6 = x2FFE
                                                ; run 322: M[x2FFE] = x8001
                                                ; run 323: MDR = x3016
                                                ; run 324: MAR = x2FFC, R6 = x2FFC
                                                ; run 329: M[x2FFC] = x3016
                                                ; run 330: MAR = x0202
                                                ; run 335: MDR = x1200
                                                ; run 336: PC = x1200
   ADD R1, R1, R3   ; lineAddress = x3016 ; run 148: R1 = x12, P = 1  ; run 206: R1 = x12 + x11 = x23, P = 1
                    ; run 264: R1 = x23 + x39 = x5c, P = 1
   ADD R0, R0, #1   ; run 158: R0 = xC001, N = 1; run 216: R0 = xC002, N = 1
                    ; run 274: R0 = xC003, N = 1
   ADD R2, R2, #-1  ; run 168: R2 = x13 = 19D, P = 1; run 226: R2 = x12 = 18D, P = 1
                    ; run 284: R2 = x11 = 17D, P = 1; 
   BRZP LOOP        ; run ; run 237: PC = x3014
                    ; run 295: PC = x3014

;store result at xC014
   LEA R2, ADR3
   LDW R0, R2, #0
   STW R1, R0, #0

; 1. test protection exception
   AND R0, R0, #0
   STW R2, R0, #0

; 2. test unaligned access exception
   ;ADD R0, R0, #3
   ;STW R1, R0, #0

; 3. test unknown opcode exception
   ;.FILL xA000

   HALT

ADR1 .FILL x4000
ADR2 .FILL xC000
ADR3 .FILL xC014
.END
