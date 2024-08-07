; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --version 2
; Test memory sanitizer instrumentation for Arm NEON VST instructions, with
; origin tracking. These tests are deliberately shorter than neon_vst.ll, due
; to the verbosity of the output.
;
; RUN: opt < %s -passes=msan -msan-track-origins=2 -S | FileCheck %s
;
; Forked from llvm/test/CodeGen/AArch64/arm64-st1.ll

target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64--linux-android9001"

; -----------------------------------------------------------------------------------------------------------------------------------------------

define void @st2_16b(<16 x i8> %A, <16 x i8> %B, ptr %P) nounwind sanitize_memory {
;
;
; CHECK-LABEL: define void @st2_16b
; CHECK-SAME: (<16 x i8> [[A:%.*]], <16 x i8> [[B:%.*]], ptr [[P:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:    [[TMP1:%.*]] = load i64, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 32) to ptr), align 8
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 32) to ptr), align 4
; CHECK-NEXT:    [[TMP3:%.*]] = load <16 x i8>, ptr @__msan_param_tls, align 8
; CHECK-NEXT:    [[TMP4:%.*]] = load i32, ptr @__msan_param_origin_tls, align 4
; CHECK-NEXT:    [[TMP5:%.*]] = load <16 x i8>, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 16) to ptr), align 8
; CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 16) to ptr), align 4
; CHECK-NEXT:    call void @llvm.donothing()
; CHECK-NEXT:    [[TMP7:%.*]] = ptrtoint ptr [[P]] to i64
; CHECK-NEXT:    [[TMP8:%.*]] = xor i64 [[TMP7]], 193514046488576
; CHECK-NEXT:    [[TMP9:%.*]] = inttoptr i64 [[TMP8]] to ptr
; CHECK-NEXT:    [[TMP10:%.*]] = add i64 [[TMP8]], 35184372088832
; CHECK-NEXT:    [[TMP11:%.*]] = and i64 [[TMP10]], -4
; CHECK-NEXT:    [[TMP12:%.*]] = inttoptr i64 [[TMP11]] to ptr
; CHECK-NEXT:    [[TMP13:%.*]] = bitcast <16 x i8> [[TMP5]] to i128
; CHECK-NEXT:    [[TMP14:%.*]] = icmp ne i128 [[TMP13]], 0
; CHECK-NEXT:    [[TMP15:%.*]] = select i1 [[TMP14]], i32 [[TMP6]], i32 [[TMP4]]
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP12]], align 4
; CHECK-NEXT:    [[TMP16:%.*]] = getelementptr i32, ptr [[TMP12]], i32 1
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP16]], align 4
; CHECK-NEXT:    [[TMP17:%.*]] = getelementptr i32, ptr [[TMP12]], i32 2
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP17]], align 4
; CHECK-NEXT:    [[TMP18:%.*]] = getelementptr i32, ptr [[TMP12]], i32 3
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP18]], align 4
; CHECK-NEXT:    [[TMP19:%.*]] = getelementptr i32, ptr [[TMP12]], i32 4
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP19]], align 4
; CHECK-NEXT:    [[TMP20:%.*]] = getelementptr i32, ptr [[TMP12]], i32 5
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP20]], align 4
; CHECK-NEXT:    [[TMP21:%.*]] = getelementptr i32, ptr [[TMP12]], i32 6
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP21]], align 4
; CHECK-NEXT:    [[TMP22:%.*]] = getelementptr i32, ptr [[TMP12]], i32 7
; CHECK-NEXT:    store i32 [[TMP15]], ptr [[TMP22]], align 4
; CHECK-NEXT:    [[_MSCMP:%.*]] = icmp ne i64 [[TMP1]], 0
; CHECK-NEXT:    br i1 [[_MSCMP]], label [[TMP23:%.*]], label [[TMP24:%.*]], !prof [[PROF0:![0-9]+]]
; CHECK:       23:
; CHECK-NEXT:    call void @__msan_warning_with_origin_noreturn(i32 [[TMP2]]) #[[ATTR4:[0-9]+]]
; CHECK-NEXT:    unreachable
; CHECK:       24:
; CHECK-NEXT:    call void @llvm.aarch64.neon.st2.v16i8.p0(<16 x i8> [[A]], <16 x i8> [[B]], ptr [[P]])
; CHECK-NEXT:    call void @llvm.aarch64.neon.st2.v16i8.p0(<16 x i8> [[TMP3]], <16 x i8> [[TMP5]], ptr [[TMP9]])
; CHECK-NEXT:    ret void
;
  call void @llvm.aarch64.neon.st2.v16i8.p0(<16 x i8> %A, <16 x i8> %B, ptr %P)
  ret void
}

define void @st3_16b(<16 x i8> %A, <16 x i8> %B, <16 x i8> %C, ptr %P) nounwind sanitize_memory {
;
;
; CHECK-LABEL: define void @st3_16b
; CHECK-SAME: (<16 x i8> [[A:%.*]], <16 x i8> [[B:%.*]], <16 x i8> [[C:%.*]], ptr [[P:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[TMP1:%.*]] = load i64, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 48) to ptr), align 8
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 48) to ptr), align 4
; CHECK-NEXT:    [[TMP3:%.*]] = load <16 x i8>, ptr @__msan_param_tls, align 8
; CHECK-NEXT:    [[TMP4:%.*]] = load i32, ptr @__msan_param_origin_tls, align 4
; CHECK-NEXT:    [[TMP5:%.*]] = load <16 x i8>, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 16) to ptr), align 8
; CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 16) to ptr), align 4
; CHECK-NEXT:    [[TMP7:%.*]] = load <16 x i8>, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 32) to ptr), align 8
; CHECK-NEXT:    [[TMP8:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 32) to ptr), align 4
; CHECK-NEXT:    call void @llvm.donothing()
; CHECK-NEXT:    [[TMP9:%.*]] = ptrtoint ptr [[P]] to i64
; CHECK-NEXT:    [[TMP10:%.*]] = xor i64 [[TMP9]], 193514046488576
; CHECK-NEXT:    [[TMP11:%.*]] = inttoptr i64 [[TMP10]] to ptr
; CHECK-NEXT:    [[TMP12:%.*]] = add i64 [[TMP10]], 35184372088832
; CHECK-NEXT:    [[TMP13:%.*]] = and i64 [[TMP12]], -4
; CHECK-NEXT:    [[TMP14:%.*]] = inttoptr i64 [[TMP13]] to ptr
; CHECK-NEXT:    [[TMP15:%.*]] = bitcast <16 x i8> [[TMP5]] to i128
; CHECK-NEXT:    [[TMP16:%.*]] = icmp ne i128 [[TMP15]], 0
; CHECK-NEXT:    [[TMP17:%.*]] = select i1 [[TMP16]], i32 [[TMP6]], i32 [[TMP4]]
; CHECK-NEXT:    [[TMP18:%.*]] = bitcast <16 x i8> [[TMP7]] to i128
; CHECK-NEXT:    [[TMP19:%.*]] = icmp ne i128 [[TMP18]], 0
; CHECK-NEXT:    [[TMP20:%.*]] = select i1 [[TMP19]], i32 [[TMP8]], i32 [[TMP17]]
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP14]], align 4
; CHECK-NEXT:    [[TMP21:%.*]] = getelementptr i32, ptr [[TMP14]], i32 1
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP21]], align 4
; CHECK-NEXT:    [[TMP22:%.*]] = getelementptr i32, ptr [[TMP14]], i32 2
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP22]], align 4
; CHECK-NEXT:    [[TMP23:%.*]] = getelementptr i32, ptr [[TMP14]], i32 3
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP23]], align 4
; CHECK-NEXT:    [[TMP24:%.*]] = getelementptr i32, ptr [[TMP14]], i32 4
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP24]], align 4
; CHECK-NEXT:    [[TMP25:%.*]] = getelementptr i32, ptr [[TMP14]], i32 5
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP25]], align 4
; CHECK-NEXT:    [[TMP26:%.*]] = getelementptr i32, ptr [[TMP14]], i32 6
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP26]], align 4
; CHECK-NEXT:    [[TMP27:%.*]] = getelementptr i32, ptr [[TMP14]], i32 7
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP27]], align 4
; CHECK-NEXT:    [[TMP28:%.*]] = getelementptr i32, ptr [[TMP14]], i32 8
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP28]], align 4
; CHECK-NEXT:    [[TMP29:%.*]] = getelementptr i32, ptr [[TMP14]], i32 9
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP29]], align 4
; CHECK-NEXT:    [[TMP30:%.*]] = getelementptr i32, ptr [[TMP14]], i32 10
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP30]], align 4
; CHECK-NEXT:    [[TMP31:%.*]] = getelementptr i32, ptr [[TMP14]], i32 11
; CHECK-NEXT:    store i32 [[TMP20]], ptr [[TMP31]], align 4
; CHECK-NEXT:    [[_MSCMP:%.*]] = icmp ne i64 [[TMP1]], 0
; CHECK-NEXT:    br i1 [[_MSCMP]], label [[TMP32:%.*]], label [[TMP33:%.*]], !prof [[PROF0]]
; CHECK:       32:
; CHECK-NEXT:    call void @__msan_warning_with_origin_noreturn(i32 [[TMP2]]) #[[ATTR4]]
; CHECK-NEXT:    unreachable
; CHECK:       33:
; CHECK-NEXT:    call void @llvm.aarch64.neon.st3.v16i8.p0(<16 x i8> [[A]], <16 x i8> [[B]], <16 x i8> [[C]], ptr [[P]])
; CHECK-NEXT:    call void @llvm.aarch64.neon.st3.v16i8.p0(<16 x i8> [[TMP3]], <16 x i8> [[TMP5]], <16 x i8> [[TMP7]], ptr [[TMP11]])
; CHECK-NEXT:    ret void
;
  call void @llvm.aarch64.neon.st3.v16i8.p0(<16 x i8> %A, <16 x i8> %B, <16 x i8> %C, ptr %P)
  ret void
}

define void @st4_16b(<16 x i8> %A, <16 x i8> %B, <16 x i8> %C, <16 x i8> %D, ptr %P) nounwind sanitize_memory {
;
;
; CHECK-LABEL: define void @st4_16b
; CHECK-SAME: (<16 x i8> [[A:%.*]], <16 x i8> [[B:%.*]], <16 x i8> [[C:%.*]], <16 x i8> [[D:%.*]], ptr [[P:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[TMP1:%.*]] = load i64, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 64) to ptr), align 8
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 64) to ptr), align 4
; CHECK-NEXT:    [[TMP3:%.*]] = load <16 x i8>, ptr @__msan_param_tls, align 8
; CHECK-NEXT:    [[TMP4:%.*]] = load i32, ptr @__msan_param_origin_tls, align 4
; CHECK-NEXT:    [[TMP5:%.*]] = load <16 x i8>, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 16) to ptr), align 8
; CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 16) to ptr), align 4
; CHECK-NEXT:    [[TMP7:%.*]] = load <16 x i8>, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 32) to ptr), align 8
; CHECK-NEXT:    [[TMP8:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 32) to ptr), align 4
; CHECK-NEXT:    [[TMP9:%.*]] = load <16 x i8>, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_tls to i64), i64 48) to ptr), align 8
; CHECK-NEXT:    [[TMP10:%.*]] = load i32, ptr inttoptr (i64 add (i64 ptrtoint (ptr @__msan_param_origin_tls to i64), i64 48) to ptr), align 4
; CHECK-NEXT:    call void @llvm.donothing()
; CHECK-NEXT:    [[TMP11:%.*]] = ptrtoint ptr [[P]] to i64
; CHECK-NEXT:    [[TMP12:%.*]] = xor i64 [[TMP11]], 193514046488576
; CHECK-NEXT:    [[TMP13:%.*]] = inttoptr i64 [[TMP12]] to ptr
; CHECK-NEXT:    [[TMP14:%.*]] = add i64 [[TMP12]], 35184372088832
; CHECK-NEXT:    [[TMP15:%.*]] = and i64 [[TMP14]], -4
; CHECK-NEXT:    [[TMP16:%.*]] = inttoptr i64 [[TMP15]] to ptr
; CHECK-NEXT:    [[TMP17:%.*]] = bitcast <16 x i8> [[TMP5]] to i128
; CHECK-NEXT:    [[TMP18:%.*]] = icmp ne i128 [[TMP17]], 0
; CHECK-NEXT:    [[TMP19:%.*]] = select i1 [[TMP18]], i32 [[TMP6]], i32 [[TMP4]]
; CHECK-NEXT:    [[TMP20:%.*]] = bitcast <16 x i8> [[TMP7]] to i128
; CHECK-NEXT:    [[TMP21:%.*]] = icmp ne i128 [[TMP20]], 0
; CHECK-NEXT:    [[TMP22:%.*]] = select i1 [[TMP21]], i32 [[TMP8]], i32 [[TMP19]]
; CHECK-NEXT:    [[TMP23:%.*]] = bitcast <16 x i8> [[TMP9]] to i128
; CHECK-NEXT:    [[TMP24:%.*]] = icmp ne i128 [[TMP23]], 0
; CHECK-NEXT:    [[TMP25:%.*]] = select i1 [[TMP24]], i32 [[TMP10]], i32 [[TMP22]]
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP16]], align 4
; CHECK-NEXT:    [[TMP26:%.*]] = getelementptr i32, ptr [[TMP16]], i32 1
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP26]], align 4
; CHECK-NEXT:    [[TMP27:%.*]] = getelementptr i32, ptr [[TMP16]], i32 2
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP27]], align 4
; CHECK-NEXT:    [[TMP28:%.*]] = getelementptr i32, ptr [[TMP16]], i32 3
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP28]], align 4
; CHECK-NEXT:    [[TMP29:%.*]] = getelementptr i32, ptr [[TMP16]], i32 4
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP29]], align 4
; CHECK-NEXT:    [[TMP30:%.*]] = getelementptr i32, ptr [[TMP16]], i32 5
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP30]], align 4
; CHECK-NEXT:    [[TMP31:%.*]] = getelementptr i32, ptr [[TMP16]], i32 6
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP31]], align 4
; CHECK-NEXT:    [[TMP32:%.*]] = getelementptr i32, ptr [[TMP16]], i32 7
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP32]], align 4
; CHECK-NEXT:    [[TMP33:%.*]] = getelementptr i32, ptr [[TMP16]], i32 8
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP33]], align 4
; CHECK-NEXT:    [[TMP34:%.*]] = getelementptr i32, ptr [[TMP16]], i32 9
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP34]], align 4
; CHECK-NEXT:    [[TMP35:%.*]] = getelementptr i32, ptr [[TMP16]], i32 10
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP35]], align 4
; CHECK-NEXT:    [[TMP36:%.*]] = getelementptr i32, ptr [[TMP16]], i32 11
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP36]], align 4
; CHECK-NEXT:    [[TMP37:%.*]] = getelementptr i32, ptr [[TMP16]], i32 12
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP37]], align 4
; CHECK-NEXT:    [[TMP38:%.*]] = getelementptr i32, ptr [[TMP16]], i32 13
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP38]], align 4
; CHECK-NEXT:    [[TMP39:%.*]] = getelementptr i32, ptr [[TMP16]], i32 14
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP39]], align 4
; CHECK-NEXT:    [[TMP40:%.*]] = getelementptr i32, ptr [[TMP16]], i32 15
; CHECK-NEXT:    store i32 [[TMP25]], ptr [[TMP40]], align 4
; CHECK-NEXT:    [[_MSCMP:%.*]] = icmp ne i64 [[TMP1]], 0
; CHECK-NEXT:    br i1 [[_MSCMP]], label [[TMP41:%.*]], label [[TMP42:%.*]], !prof [[PROF0]]
; CHECK:       41:
; CHECK-NEXT:    call void @__msan_warning_with_origin_noreturn(i32 [[TMP2]]) #[[ATTR4]]
; CHECK-NEXT:    unreachable
; CHECK:       42:
; CHECK-NEXT:    call void @llvm.aarch64.neon.st4.v16i8.p0(<16 x i8> [[A]], <16 x i8> [[B]], <16 x i8> [[C]], <16 x i8> [[D]], ptr [[P]])
; CHECK-NEXT:    call void @llvm.aarch64.neon.st4.v16i8.p0(<16 x i8> [[TMP3]], <16 x i8> [[TMP5]], <16 x i8> [[TMP7]], <16 x i8> [[TMP9]], ptr [[TMP13]])
; CHECK-NEXT:    ret void
;
  call void @llvm.aarch64.neon.st4.v16i8.p0(<16 x i8> %A, <16 x i8> %B, <16 x i8> %C, <16 x i8> %D, ptr %P)
  ret void
}

declare void @llvm.aarch64.neon.st2.v16i8.p0(<16 x i8>, <16 x i8>, ptr) nounwind sanitize_memory readonly
declare void @llvm.aarch64.neon.st3.v16i8.p0(<16 x i8>, <16 x i8>, <16 x i8>, ptr) nounwind sanitize_memory readonly
declare void @llvm.aarch64.neon.st4.v16i8.p0(<16 x i8>, <16 x i8>, <16 x i8>, <16 x i8>, ptr) nounwind sanitize_memory readonly
