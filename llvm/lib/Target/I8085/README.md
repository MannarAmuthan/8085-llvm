
# **I8085 backend**

This is the experimental LLVM backend for the  [intel 8085 microprocessor](https://en.wikipedia.org/wiki/Intel_8085).


## Contents

[1. Implemented Functionalities](#implemented)

[2. Some implementation notes](#notes)

[3. Correctness](#correctness)

[4. Disclaimer](#disclaimer)

[5. Next on priority](#priority)

[6. Credits](#credits)

## <a id="implemented">**Implemented Functionalities:**

Currently basic constructs such as , loading and storing variables, function call, logical operations (and, or, xor) and two arithmetic operations such as addition and subtraction are implemented. And those are also only for i8 and i16 integer types alone. Still far way to go.

**Example:**
``` c
short ret(short  a,short  b){
return a+b;
}
```
3. Compiling above file using clang produces below one.

```ll
define signext i16 @ret(i16 signext %0, i16 signext %1) #0 {
  %3 = alloca i16, align 2
  %4 = alloca i16, align 2
  store i16 %0, i16* %3, align 2
  store i16 %1, i16* %4, align 2
  %5 = load i16, i16* %3, align 2
  %6 = sext i16 %5 to i32
  %7 = load i16, i16* %4, align 2
  %8 = sext i16 %7 to i32
  %9 = add nsw i32 %6, %8
  %10 = trunc i32 %9 to i16
  ret i16 %10
}
```
4. And when input this to the llc tool , this backend produces below 8085 assembly
```assembly
		.text
	.file	"test.c"
	.globl	ret                             ; -- Begin function ret
	.p2align	1
	.type	ret,@function
ret:                                    ; @ret
; %bb.0:
	LXI H, 0
	DAD	SP
	MOV	A, L
	MVI	L, 4
	SUB L
	MOV	L, A
	MOV	A, H
	MVI	H, 0
	SBB H
	MOV	H, A
	SPHL
	LXI H, 9
	DAD	SP
	MOV B, M
	LXI H, 8
	DAD	SP
	MOV C, M
	LXI H, 7
	DAD	SP
	MOV D, M
	LXI H, 6
	DAD	SP
	MOV E, M
	LXI H, 3
	DAD	SP
	MOV M, D
	LXI H, 2
	DAD	SP
	MOV M, E
	LXI H, 1
	DAD	SP
	MOV M, B
	LXI H, 0
	DAD	SP
	MOV M, C
	LXI H, 3
	DAD	SP
	MOV B, M
	LXI H, 2
	DAD	SP
	MOV C, M
	LXI H, 1
	DAD	SP
	MOV D, M
	LXI H, 0
	DAD	SP
	MOV E, M
	MOV	A, C
	ADD E
	MOV	C, A
	MOV	A, B
	ADC D
	MOV	B, A
	ret
.Lfunc_end0:
	.size	ret, .Lfunc_end0-ret
                                        ; -- End function
```

**Some more examples:**

Input:
```ll
define i16 @foo(i16,i16) {
  %3 = add i16 %0, %1
  %4 = add i16 %3, %3
  %5 = add i16 %4, %4
  %6 = sub i16 %5, %4
  %7 = sub i16 %6, %5
  ret i16 %7
}
```

Output:
```assembly
		.text
	.file	"test.ll"
	.globl	foo                             ; -- Begin function foo
	.p2align	1
	.type	foo,@function
foo:                                    ; @foo
; %bb.0:
	LXI H, 5
	DAD	SP
	MOV D, M
	LXI H, 4
	DAD	SP
	MOV E, M
	LXI H, 3
	DAD	SP
	MOV B, M
	LXI H, 2
	DAD	SP
	MOV C, M
	MOV	A, C
	ADD E
	MOV	C, A
	MOV	A, B
	ADC D
	MOV	B, A
	MOV	A, C
	ADD C
	MOV	C, A
	MOV	A, B
	ADC B
	MOV	B, A
	MOV	D, B
	MOV	A, E
	ADD E
	MOV	E, A
	MOV	A, D
	ADC D
	MOV	D, A
	MOV	A, C
	SUB E
	MOV	C, A
	MOV	A, B
	SBB D
	MOV	B, A
	ret
.Lfunc_end0:
	.size	foo, .Lfunc_end0-foo
                                        ; -- End function

```

Input:
```ll
define i16 @functiontwo(i16,i16,i16){
  ret i16 %2
}

define i16 @functionone(i16) {
    %2 = alloca i16, align 1
    store i16 10000, i16* %2, align 1
    %3 = load i16, i16* %2, align 1

    %4 = alloca i16, align 1
    store i16 10000, i16* %4, align 1
    %5 = load i16, i16* %4, align 1

    %6 = alloca i16, align 1
    store i16 %0, i16* %6, align 1
    %7 = load i16, i16* %6, align 1


    %8 = call i16 @functiontwo(i16 %3,i16 %5,i16 %7)
    ret i16 %8
}
```

Output:
```assembly
	.text
	.file	"test.ll"
	.globl	functiontwo                     ; -- Begin function functiontwo
	.p2align	1
	.type	functiontwo,@function
functiontwo:                            ; @functiontwo
; %bb.0:
	LXI H, 7
	DAD	SP
	MOV B, M
	LXI H, 6
	DAD	SP
	MOV C, M
	ret
.Lfunc_end0:
	.size	functiontwo, .Lfunc_end0-functiontwo
                                        ; -- End function
	.globl	functionone                     ; -- Begin function functionone
	.p2align	1
	.type	functionone,@function
functionone:                            ; @functionone
; %bb.0:
	LXI H, 0
	DAD	SP
	MOV	A, L
	MVI	L, 10
	SUB L
	MOV	L, A
	MOV	A, H
	MVI	H, 0
	SBB H
	MOV	H, A
	SPHL
	MVI	B, 39
	MVI	C, 16
	LXI H, 9
	DAD	SP
	MOV M, B
	LXI H, 8
	DAD	SP
	MOV M, C
	LXI H, 7
	DAD	SP
	MOV M, B
	LXI H, 6
	DAD	SP
	MOV M, C
	LXI H, 13
	DAD	SP
	MOV D, M
	LXI H, 12
	DAD	SP
	MOV E, M
	LXI H, 6
	DAD	SP
	MOV M, D
	LXI H, 5
	DAD	SP
	MOV M, E
	LXI H, 4
	DAD	SP
	MOV M, B
	LXI H, 3
	DAD	SP
	MOV M, C
	LXI H, 2
	DAD	SP
	MOV M, B
	LXI H, 1
	DAD	SP
	MOV M, C
	CALL functiontwo
	ret
.Lfunc_end1:
	.size	functionone, .Lfunc_end1-functionone
                                        ; -- End function
```

Input:
```ll
define i16 @logical(i16,i16) {
  %3 = xor i16 %0, %1 
  %4 = and i16 %3, %1
  %5 = or i16 %4, %0 
  ret i16 %5
}
```

Output:
```assembly
	.text
	.file	"test.ll"
	.globl	logical                         ; -- Begin function logical
	.p2align	1
	.type	logical,@function
logical:                                ; @logical
; %bb.0:
	LXI H, 0
	DAD	SP
	MOV	A, L
	MVI	L, 2
	SUB L
	MOV	L, A
	MOV	A, H
	MVI	H, 0
	SBB H
	MOV	H, A
	SPHL
	LXI H, 7
	DAD	SP
	MOV D, M
	LXI H, 6
	DAD	SP
	MOV E, M
	LXI H, 5
	DAD	SP
	MOV B, M
	LXI H, 4
	DAD	SP
	MOV C, M
	LXI H, 1
	DAD	SP
	MOV M, B
	LXI H, 0
	DAD	SP
	MOV M, C
	MOV	A, C
	XRA E
	MOV	C, A
	MOV	A, B
	XRA D
	MOV	B, A
	MOV	A, C
	ANA E
	MOV	C, A
	MOV	A, B
	ANA D
	MOV	B, A
	LXI H, 1
	DAD	SP
	MOV D, M
	LXI H, 0
	DAD	SP
	MOV E, M
	MOV	A, C
	ORA E
	MOV	C, A
	MOV	A, B
	ORA D
	MOV	B, A
	ret
.Lfunc_end0:
	.size	logical, .Lfunc_end0-logical
                                        ; -- End function
```
## <a id="notes">**Some implementation note:**

1. Arguments are passed via stack, and return value is stored in register.
2. 8 bit integers are returned via A reg and 16 bit integers are returned via BC reg pair.
3. HL reg is used as base pointer here.


## <a id="correctness">**Correctness:**

The output assembly is tested in open source 8085 simulators and assemblers.  Codegen Tests are located [here](llvm/test/CodeGen/I8085).

## <a id="disclaimer">**Disclaimer:**

This 8085 backend is purely experimental and never tested with original hardware and never used in productional use cases. 

## <a id="priority">**Next on priority:**
1.  When running test cases with -verifymachineinstr tag, tests are failing, so need to fix this.
2.  Implementing branch instructions.
3. Implement shift operations.
4. Adding 32 bit integer support.
5. Adding more arithmetic operations and floating point types.


## <a id="credits"> **Thanks and Credits:**

This backend is inspired by LLVM backend for AVR 8 bit microcontroller. Some of the functionalities here is referred from there. 