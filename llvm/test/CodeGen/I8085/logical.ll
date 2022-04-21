; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s

define i8 @functionone(i8,i8) {
; CHECK-LABEL:   functionone: 
; CHECK: LXI H, 3
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: LXI H, 2
; CHECK: DAD	SP
; CHECK: MOV C, M
; CHECK: MOV	D, C
; CHECK: MOV	A, D
; CHECK: XRA B
; CHECK: MOV	D, A
; CHECK: MOV	A, D
; CHECK: ANA B
; CHECK: MOV	D, A
; CHECK: MOV	A, D
; CHECK: ORA C
; CHECK: MOV	D, A
; CHECK: MOV	A, D
; CHECK: ret
  %3 = xor i8 %0, %1
  %4 = and i8 %3, %1
  %5 = or i8 %4, %0
  ret i8 %5
}

define i16 @functiontwo(i16,i16) {
; CHECK-LABEL:   functiontwo:   
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV	A, L
; CHECK: MVI	L, 2
; CHECK: SUB L
; CHECK: MOV	L, A
; CHECK: MOV	A, H
; CHECK: MVI	H, 0
; CHECK: SBB H
; CHECK: MOV	H, A
; CHECK: SPHL
; CHECK: LXI H, 6
; CHECK: DAD	SP
; CHECK: MOV D, M
; CHECK: LXI H, 7
; CHECK: DAD	SP
; CHECK: MOV E, M
; CHECK: LXI H, 4
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: LXI H, 5
; CHECK: DAD	SP
; CHECK: MOV C, M
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV M, B
; CHECK: LXI H, 1
; CHECK: DAD	SP
; CHECK: MOV M, C
; CHECK: MOV	A, C
; CHECK: XRA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: XRA D
; CHECK: MOV	B, A
; CHECK: MOV	A, C
; CHECK: ANA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ANA D
; CHECK: MOV	B, A
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV D, M
; CHECK: LXI H, 1
; CHECK: DAD	SP
; CHECK: MOV E, M
; CHECK: MOV	A, C
; CHECK: ORA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ORA D
; CHECK: MOV	B, A
; CHECK: ret

  %3 = xor i16 %0, %1 
  %4 = and i16 %3, %1
  %5 = or i16 %4, %0 
  ret i16 %5
}

define i8 @functionthree(i8,i8) {
; CHECK-LABEL:   functionthree: 
; CHECK: LXI H, 3
; CHECK: DAD	SP
; CHECK: MOV C, M
; CHECK: LXI H, 2
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: MOV	A, B
; CHECK: XRA C
; CHECK: MOV	B, A
; CHECK: MOV	A, B
; CHECK: ANI 40
; CHECK: MOV	B, A
; CHECK: MOV	A, B
; CHECK: ORI 80
; CHECK: MOV	B, A
; CHECK: MOV	A, B
; CHECK: XRI 111
; CHECK: MOV	B, A
; CHECK: MOV	A, B
; CHECK: ret    
  %3 = xor i8 %0, %1
  %4 = and i8 %3, 40
  %5 = or i8 %4, 80
  %6 = xor i8 %5, 111
  ret i8 %6
}

define i16 @functionfour(i16,i16) {
; CHECK-LABEL:   functionfour:     
; CHECK: LXI H, 4
; CHECK: DAD	SP
; CHECK: MOV D, M
; CHECK: LXI H, 5
; CHECK: DAD	SP
; CHECK: MOV E, M
; CHECK: LXI H, 2
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: LXI H, 3
; CHECK: DAD	SP
; CHECK: MOV C, M
; CHECK: MOV	A, C
; CHECK: XRA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: XRA D
; CHECK: MOV	B, A
; CHECK: MVI	D, 140
; CHECK: MVI	E, 64
; CHECK: MOV	A, C
; CHECK: ANA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ANA D
; CHECK: MOV	B, A
; CHECK: MVI	D, 48
; CHECK: MVI	E, 57
; CHECK: MOV	A, C
; CHECK: ORA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ORA D
; CHECK: MOV	B, A
; CHECK: MVI	D, 213
; CHECK: MVI	E, 150
; CHECK: MOV	A, C
; CHECK: XRA E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: XRA D
; CHECK: MOV	B, A
; CHECK: ret

  %3 = xor i16 %0, %1 
  %4 = and i16 %3, 40000 
  %5 = or i16 %4, 12345 
  %6 = xor i16 %5, 54678
  ret i16 %6
}