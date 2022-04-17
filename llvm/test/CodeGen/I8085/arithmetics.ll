; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s

define i16 @add_sub_1(i16,i16) {
; CHECK-LABEL:   add_sub_1:
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
; CHECK: ADD E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC D
; CHECK: MOV	B, A
; CHECK: MOV	A, C
; CHECK: ADD C
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC B
; CHECK: MOV	B, A
; CHECK: MOV	A, C
; CHECK: ADD C
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC B
; CHECK: MOV	B, A
; CHECK: MOV	A, C
; CHECK: ADD C
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC B
; CHECK: MOV	B, A
; CHECK: MVI	D, 0
; CHECK: MVI	E, 223
; CHECK: MOV	A, C
; CHECK: SUB E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: SBB D
; CHECK: MOV	B, A
; CHECK: ret

  %3 = add i16 %0, %1
  %4 = add i16 %3, %3
  %5 = add i16 %4, %4
  %6 = add i16 %5, %5
  %7 = sub i16 %6, 223
  ret i16 %7
}


define i16 @add_sub_2(i16,i16) {

; CHECK-LABEL:   add_sub_2:
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
; CHECK: ADD E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC D
; CHECK: MOV	B, A
; CHECK: MVI	D, 39
; CHECK: MVI	E, 16
; CHECK: MOV	A, C
; CHECK: SUB E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: SBB D
; CHECK: MOV	B, A
; CHECK: MOV	A, C
; CHECK: ADD C
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC B
; CHECK: MOV	B, A
; CHECK: MVI	D, 0
; CHECK: MVI	E, 3
; CHECK: MOV	A, C
; CHECK: ADD E
; CHECK: MOV	C, A
; CHECK: MOV	A, B
; CHECK: ADC D
; CHECK: MOV	B, A
; CHECK: ret

  %3 = add i16 %0, %1 
  %4 = sub i16 %3, 10000 
  %5 = add i16 %4, %4 
  %6 = add i16 3, %5 
  ret i16 %6
}

define i8 @add_sub_3(i8,i8) {
; CHECK-LABEL:   add_sub_3:
; CHECK: LXI H, 3
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: LXI H, 2
; CHECK: DAD	SP
; CHECK: MOV C, M
; CHECK: MOV	A, C
; CHECK: ADD B
; CHECK: MOV	C, A
; CHECK: MVI	B, 58
; CHECK: MOV	A, C
; CHECK: ADD B
; CHECK: MOV	C, A
; CHECK: MOV	A, C
; CHECK: ret

  %3 = add i8 %0, %1 ; 48
  %4 = sub i8 100, %3 ; 52
  %5 = sub i8 54, %4 ; 2
  %6 = add i8 101, %5 ; 103
  %7 = add i8 3, %6 ;  106
  ret i8 %7
}


define i8 @add_sub_4(i8,i8) {
; CHECK-LABEL:   add_sub_4:
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
; CHECK: MVI	B, 100
; CHECK: LXI H, 1
; CHECK: DAD	SP
; CHECK: MOV M, B
; CHECK: LXI H, 4
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: MVI	C, -95
; CHECK: MOV	A, C
; CHECK: SUB B
; CHECK: MOV	C, A
; CHECK: LXI H, 0
; CHECK: DAD	SP
; CHECK: MOV M, C
; CHECK: MOV	A, C

  %3 = alloca i8, align 1
  store i8 100, i8* %3, align 1
  %4 = load i8, i8* %3, align 1
  
  %5 = alloca i8, align 1
  store i8 %0, i8* %5, align 1
  %6 = load i8, i8* %5, align 1
  
  %7 = add i8 %4, %6
  %8 = sub i8 5 , %7 

  %9 = alloca i8, align 1
  store i8 %8, i8* %9, align 1
  %10 = load i8, i8* %9, align 1
  
  ret i8 %10
}


