; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s

define i16 @argpass16(i16,i16,i16) #0  {
; CHECK-LABEL:   argpass16:     
; CHECK: LXI H, 6
; CHECK: DAD	SP
; CHECK: MOV B, M
; CHECK: LXI H, 7
; CHECK: DAD	SP
; CHECK: MOV C, M

  %4 = alloca i16, align 1
  store i16 %2, i16* %4, align 1
  %5 = load i16, i16* %4, align 1
  ret i16 %5
}


define i8 @argpass8(i8,i8,i8)  {
; CHECK-LABEL:   argpass8:     
; CHECK: LXI H, 4
; CHECK: DAD	SP
; CHECK: MOV A, M
; CHECK: ret

  %4 = alloca i8, align 1
  store i8 %2, i8* %4, align 1
  %5 = load i8, i8* %4, align 1
  ret i8 %5
}