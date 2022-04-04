; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s

define i8 @load_test() #0 {
;  CHECK-LABEL:   load_test:     
;  CHECK: LXI H, 0
;  CHECK: DAD	SP
;  CHECK: SHLD 65530
;  CHECK: MOV A, L
;  CHECK: MVI L, 2
;  CHECK: SUB L
;  CHECK: MOV L, A
;  CHECK: MOV A, H
;  CHECK: MVI H, 0
;  CHECK: SBB H
;  CHECK: MOV H, A
;  CHECK: SPHL
;  CHECK: MVI B, 8
;  CHECK: LXI H, 1
;  CHECK: DAD SP
;  CHECK: MOV M, B
;  CHECK: MOV A, B
;  CHECK: LHLD 65530
;  CHECK: SPHL
;  CHECK: ret

  %1 = alloca i8, align 2
  store i8 8, i8* %1, align 2
  %2 = load i8, i8* %1, align 2
  ret i8 %2
}