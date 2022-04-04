; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s


define i8 @store_test() #0 {
;  CHECK-LABEL:   store_test:     
;  CHECK: LXI H, 0
;  CHECK: DAD	SP
;  CHECK: SHLD 65530
;  CHECK: MOV	A, L
;  CHECK: MVI	L, 3
;  CHECK: SUB L
;  CHECK: MOV	L, A
;  CHECK: MOV	A, H
;  CHECK: MVI	H, 0
;  CHECK: SBB H
;  CHECK: MOV	H, A
;  CHECK: SPHL
;  CHECK: MVI	B, 54
;  CHECK: LXI H, 2
;  CHECK: DAD	SP
;  CHECK: MOV M, B
;  CHECK: MVI	B, 8
;  CHECK: LXI H, 3
;  CHECK: DAD	SP
;  CHECK: MOV M, B
;  CHECK: MVI	B, 21
;  CHECK: LXI H, 1
;  CHECK: DAD	SP
;  CHECK: MOV M, B
;  CHECK: MVI	A, 100
;  CHECK: LHLD 65530
;  CHECK: SPHL
;  CHECK: ret

  %1 = alloca i8, align 1
  store i8 8, i8* %1, align 1
  %2 = alloca i8, align 1
  store i8 54, i8* %2, align 1
  %3 = alloca i8, align 1
  store i8 21, i8* %3, align 1
  ret i8 100
}