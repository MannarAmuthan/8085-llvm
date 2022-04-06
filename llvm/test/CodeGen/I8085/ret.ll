; RUN: llc -mattr=i8085,sram < %s -march=i8085  | FileCheck %s

define void @retvoid(i8* %x) {
; CHECK-LABEL: retvoid:
; CHECK: ret
  ret void
}


define i8 @reteight() #0 {
; CHECK-LABEL: reteight:
; CHECK: MVI	B, 56
; CHECK: LXI H, 3
; CHECK: DAD	SP
; CHECK: MOV M, B
; CHECK: ret
  ret i8 56
}
