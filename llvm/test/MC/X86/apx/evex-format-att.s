# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s

## MRMDestMem

# CHECK: vextractf32x4	$1, %zmm0, (%r16,%r17)
# CHECK: encoding: [0x62,0xfb,0x79,0x48,0x19,0x04,0x08,0x01]
         vextractf32x4	$1, %zmm0, (%r16,%r17)

# CHECK: addq	%r16, 123(%r17), %r18
# CHECK: encoding: [0x62,0xec,0xec,0x10,0x01,0x41,0x7b]
         addq	%r16, 123(%r17), %r18

## MRMDestMemCC

# CHECK: cfcmovbq %r16, 123(%r17,%r18,4)
# CHECK: encoding: [0x62,0xec,0xf8,0x0c,0x42,0x44,0x91,0x7b]
         cfcmovbq %r16, 123(%r17,%r18,4)

## MRMSrcMem

# CHECK: vbroadcasti32x4	(%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xfa,0x79,0x48,0x5a,0x04,0x08]
         vbroadcasti32x4	(%r16,%r17), %zmm0

# CHECK: subq	123(%r16), %r17, %r18
# CHECK: encoding: [0x62,0xec,0xec,0x10,0x2b,0x48,0x7b]
         subq	123(%r16), %r17, %r18

## MRMSrcMemCC

# CHECK: cfcmovbq	123(%r16,%r17,4), %r18
# CHECK: encoding: [0x62,0xec,0xf8,0x08,0x42,0x54,0x88,0x7b]
         cfcmovbq	123(%r16,%r17,4), %r18

# CHECK: cfcmovbq	123(%r16,%r17,4), %r18, %r19
# CHECK: encoding: [0x62,0xec,0xe0,0x14,0x42,0x54,0x88,0x7b]
         cfcmovbq	123(%r16,%r17,4), %r18, %r19

## MRM0m

# CHECK: vprorq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x04,0x08,0x00]
         vprorq	$0, (%r16,%r17), %zmm0

# CHECK: addq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x40,0x7b,0x7f]
         addq	$127, 123(%r16), %r17

## MRM1m

# CHECK: vprolq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x0c,0x08,0x00]
         vprolq	$0, (%r16,%r17), %zmm0

# CHECK: orq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x48,0x7b,0x7f]
         orq	$127, 123(%r16), %r17

## MRM2m

# CHECK: vpsrlq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x73,0x14,0x08,0x00]
         vpsrlq	$0, (%r16,%r17), %zmm0

# CHECK: adcq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x50,0x7b,0x7f]
         adcq	$127, 123(%r16), %r17

## MRM3m

# CHECK: vpsrldq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0x79,0x48,0x73,0x1c,0x08,0x00]
         vpsrldq	$0, (%r16,%r17), %zmm0

# CHECK: sbbq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x58,0x7b,0x7f]
         sbbq	$127, 123(%r16), %r17

## MRM4m

# CHECK: vpsraq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x72,0x24,0x08,0x00]
         vpsraq	$0, (%r16,%r17), %zmm0

# CHECK: andq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x60,0x7b,0x7f]
         andq	$127, 123(%r16), %r17

## MRM5m

# CHECK: vscatterpf0dps	(%r16,%zmm0) {%k1}
# CHECK: encoding: [0x62,0xfa,0x7d,0x49,0xc6,0x2c,0x00]
         vscatterpf0dps	(%r16,%zmm0) {%k1}

# CHECK: subq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x68,0x7b,0x7f]
         subq	$127, 123(%r16), %r17

## MRM6m

# CHECK: vpsllq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0xf9,0x48,0x73,0x34,0x08,0x00]
         vpsllq	$0, (%r16,%r17), %zmm0

# CHECK: xorq	$127, 123(%r16), %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0x70,0x7b,0x7f]
         xorq	$127, 123(%r16), %r17

## MRM7m

# CHECK: vpslldq	$0, (%r16,%r17), %zmm0
# CHECK: encoding: [0x62,0xf9,0x79,0x48,0x73,0x3c,0x08,0x00]
         vpslldq	$0, (%r16,%r17), %zmm0

# CHECK: sarq	$123, 291(%r16,%r17), %r18
# CHECK: encoding: [0x62,0xfc,0xe8,0x10,0xc1,0xbc,0x08,0x23,0x01,0x00,0x00,0x7b]
         sarq	$123, 291(%r16,%r17), %r18

## MRMDestMem4VOp3CC

# CHECK: cmpbexadd	%r18d, %r22d, 291(%r28,%r29,4)
# CHECK: encoding: [0x62,0x8a,0x69,0x00,0xe6,0xb4,0xac,0x23,0x01,0x00,0x00]
         cmpbexadd	%r18d, %r22d, 291(%r28,%r29,4)

## MRMSrcMem4VOp3

# CHECK: bzhiq	%r19, 291(%r28,%r29,4), %r23
# CHECK: encoding: [0x62,0x8a,0xe0,0x00,0xf5,0xbc,0xac,0x23,0x01,0x00,0x00]
         bzhiq	%r19, 291(%r28,%r29,4), %r23

## MRMDestReg

# CHECK: vextractps	$1, %xmm16, %r16d
# CHECK: encoding: [0x62,0xeb,0x7d,0x08,0x17,0xc0,0x01]
         vextractps	$1, %xmm16, %r16d

# CHECK: {nf}	addq	%r16, %r17
# CHECK: encoding: [0x62,0xec,0xfc,0x0c,0x01,0xc1]
         {nf}	addq	%r16, %r17

## MRMDestRegCC

# CHECK: cfcmovbq	%r16, %r17
# CHECK: encoding: [0x62,0xec,0xfc,0x0c,0x42,0xc1]
         cfcmovbq	%r16, %r17

## MRMSrcReg

# CHECK: mulxq	%r16, %r17, %r18
# CHECK: encoding: [0x62,0xea,0xf7,0x00,0xf6,0xd0]
         mulxq	%r16, %r17, %r18

## MRMSrcRegCC

# CHECK: cfcmovbq	%r16, %r17, %r18
# CHECK: encoding: [0x62,0xec,0xec,0x14,0x42,0xc8]
         cfcmovbq	%r16, %r17, %r18

## MRMSrcReg4VOp3

# CHECK: bzhiq	%r19, %r23, %r27
# CHECK: encoding: [0x62,0x6a,0xe4,0x00,0xf5,0xdf]
         bzhiq	%r19, %r23, %r27

## MRM0r

# CHECK: addq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xc0,0x7f]
         addq	$127, %r16, %r17

## MRM1r

# CHECK: orq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xc8,0x7f]
         orq	$127, %r16, %r17

## MRM2r

# CHECK: adcq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xd0,0x7f]
         adcq	$127, %r16, %r17

## MRM3r

# CHECK: sbbq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xd8,0x7f]
         sbbq	$127, %r16, %r17

## MRM4r

# CHECK: andq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xe0,0x7f]
         andq	$127, %r16, %r17

## MRM5r

# CHECK: subq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xe8,0x7f]
         subq	$127, %r16, %r17

## MRM6r

# CHECK: xorq	$127, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0x83,0xf0,0x7f]
         xorq	$127, %r16, %r17

## MRM7r

# CHECK: sarq	$123, %r16, %r17
# CHECK: encoding: [0x62,0xfc,0xf4,0x10,0xc1,0xf8,0x7b]
         sarq	$123, %r16, %r17

## NoCD8

# CHECK: {nf}	negq	123(%r16)
# CHECK: encoding: [0x62,0xfc,0xfc,0x0c,0xf7,0x58,0x7b]
         {nf}	negq	123(%r16)

# CHECK: {evex}	notq	123(%r16)
# CHECK: encoding: [0x62,0xfc,0xfc,0x08,0xf7,0x50,0x7b]
         {evex}	notq	123(%r16)
