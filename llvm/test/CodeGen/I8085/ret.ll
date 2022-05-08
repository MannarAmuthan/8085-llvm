; RUN: llc -mattr=i8085,sram < %s -march=i8085 -verify-machineinstrs  | FileCheck %s

define void @retvoid(i8* %x) {
; CHECK-LABEL: retvoid:
; CHECK: RET
  ret void
}


define i8 @reteight() #0 {
; CHECK-LABEL: reteight:
; CHECK: MVI	A, 56
; CHECK: RET
  ret i8 56
}
