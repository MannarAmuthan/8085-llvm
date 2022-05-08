; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s

define i16 @loadtest16() #0  {
; CHECK-LABEL:   loadtest16:     
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV	A, L
; CHECK: MVI	L, 4
; CHECK: SUB L
; CHECK: MOV	L, A
; CHECK: MOV	A, H
; CHECK: MVI	H, 0
; CHECK: SBB H
; CHECK: MOV	H, A
; CHECK: SPHL
; CHECK: MVI	B, 78
; CHECK: MVI	C, 32
; CHECK: LXI H, 1
; CHECK: DAD	SP
; CHECK: MOV M, B
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV M, C
; CHECK: MVI	D, 0
; CHECK: MVI	E, 105
; CHECK: LXI H, 3
; CHECK: DAD	SP
; CHECK: MOV M, D
; CHECK: LXI H, 2
; CHECK: DAD	SP
; CHECK: MOV M, E
; CHECK: RET

  %1 = alloca i16, align 1
  store i16 105, i16* %1, align 1
  %2 = load i16, i16* %1, align 1

  %3 = alloca i16, align 1
  store i16 20000, i16* %3, align 1
  %4 = load i16, i16* %3, align 1
  ret i16 %4

}


define i8 @loadtest8()  {
; CHECK-LABEL:   loadtest8:     
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
; CHECK: MVI	B, 111
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV M, B
; CHECK: MVI	C, 55
; CHECK: LXI H, 1
; CHECK: DAD	SP
; CHECK: MOV M, C
; CHECK: MOV	A, B
; CHECK: RET
  %1 = alloca i8, align 1
  store i8 55, i8* %1, align 1
  %2 = load i8, i8* %1, align 1

  %3 = alloca i8, align 1
  store i8 111, i8* %3, align 1
  %4 = load i8, i8* %3, align 1
  ret i8 %4
}