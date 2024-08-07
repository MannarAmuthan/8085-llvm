; RUN: opt -mtriple=x86_64-- -passes=early-cse -earlycse-debug-hash < %s -S | FileCheck %s

; CHECK: @foo(<1 x i64> zeroinitializer)

define void @bar() {
entry:
  %0 = bitcast double 0.0 to x86_mmx
  %1 = call x86_mmx @foo(x86_mmx %0)
  ret void
}

declare x86_mmx @foo(x86_mmx)
