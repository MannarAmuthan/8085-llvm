# Experimental LLVM Backend for Intel 8085 architecture

For more details, current status of this backend, Please refer this [target's readme](llvm/lib/Target/I8085).

## Build

```shell
cd llvm 
mkdir build
cd build
cmake -G "Ninja" -DLLVM_TARGETS_TO_BUILD="I8085" -DCMAKE_BUILD_TYPE="Debug" -DLLVM_ENABLE_ASSERTIONS=On ../
ninja
```

## Compile LLVM IR file
Make sure that there are no attributes, metadata related to other architecture in the .ll file.

```shell
llvm/build/bin/llc fibonacci.ll -mtriple=i8085 -march=i8085
# For compiling with verify machine instructions, and print results in each pass
llvm/build/bin/llc fibonacci.ll -mtriple=i8085 -march=i8085 -verify-machineinstrs -print-after-all
```

For using other build systems and methods, please refer official LLVM Build Docs.

Example : fibonacci.c

```c
#include<stdlib.h>

int16_t fibonacci(int16_t n){
    if(n <= 1){
        return n;
    }
    return fibonacci(n-1)+fibonacci(n-2);
} 
```

```assembly
fibonacci:
LBB00:
	LXI H, 65522
	DAD SP
	SPHL
	LXI H, 17
	DAD SP
	MOV B, M
	LXI H, 16
	DAD SP
	MOV C, M
	LXI H, 11
	DAD SP
	MOV M, B
	LXI H, 10
	DAD SP
	MOV M, C
	LXI H, 15
	MOV M, C
	LXI H, 16
	MOV M, B
	MOV A, B
	ADI 128
	SBB A
	LXI H, 18
	MOV M, A
	LXI H, 17
	MOV M, A
	LXI H, 11
	MVI M, 1
	LXI H, 12
	MVI M, 0
	LXI H, 13
	MVI M, 0
	LXI H, 14
	MVI M, 0
	LXI H, 15
	MOV A, M
	LXI H, 11
	XRA M
	ANI 128
	JZ LBB01
	JMP LBB03
LBB01:
	LXI H, 11
	MOV A, M
	LXI H, 6
	DAD SP
	MOV M, A
	LXI H, 12
	MOV A, M
	LXI H, 7
	DAD SP
	MOV M, A
	LXI H, 13
	MOV A, M
	LXI H, 8
	DAD SP
	MOV M, A
	LXI H, 14
	MOV A, M
	LXI H, 9
	DAD SP
	MOV M, A
	LXI H, 15
	MOV A, M
	LXI H, 11
	MOV M, A
	LXI H, 16
	MOV A, M
	LXI H, 12
	MOV M, A
	LXI H, 17
	MOV A, M
	LXI H, 13
	MOV M, A
	LXI H, 18
	MOV A, M
	LXI H, 14
	MOV M, A
	LXI H, 15
	MOV A, M
	LXI H, 2
	DAD SP
	MOV M, A
	LXI H, 16
	MOV A, M
	LXI H, 3
	DAD SP
	MOV M, A
	LXI H, 17
	MOV A, M
	LXI H, 4
	DAD SP
	MOV M, A
	LXI H, 18
	MOV A, M
	LXI H, 5
	DAD SP
	MOV M, A
	LXI H, 6
	DAD SP
	MOV A, M
	LXI H, 15
	MOV M, A
	LXI H, 7
	DAD SP
	MOV A, M
	LXI H, 16
	MOV M, A
	LXI H, 8
	DAD SP
	MOV A, M
	LXI H, 17
	MOV M, A
	LXI H, 9
	DAD SP
	MOV A, M
	LXI H, 18
	MOV M, A
	LXI H, 11
	MOV A, M
	LXI H, 15
	SUB M
	LXI H, 11
	MOV M, A
	LXI H, 12
	MOV A, M
	LXI H, 16
	SBB M
	LXI H, 12
	MOV M, A
	LXI H, 13
	MOV A, M
	LXI H, 17
	SBB M
	LXI H, 13
	MOV M, A
	LXI H, 14
	MOV A, M
	LXI H, 18
	SBB M
	LXI H, 14
	MOV M, A
	JC LBB02
	LXI H, 6
	DAD SP
	MOV A, M
	LXI H, 11
	MOV M, A
	LXI H, 7
	DAD SP
	MOV A, M
	LXI H, 12
	MOV M, A
	LXI H, 8
	DAD SP
	MOV A, M
	LXI H, 13
	MOV M, A
	LXI H, 9
	DAD SP
	MOV A, M
	LXI H, 14
	MOV M, A
	LXI H, 2
	DAD SP
	MOV A, M
	LXI H, 15
	MOV M, A
	LXI H, 3
	DAD SP
	MOV A, M
	LXI H, 16
	MOV M, A
	LXI H, 4
	DAD SP
	MOV A, M
	LXI H, 17
	MOV M, A
	LXI H, 5
	DAD SP
	MOV A, M
	LXI H, 18
	MOV M, A
	LXI H, 15
	MOV A, M
	LXI H, 11
	CMP M
	JNZ LBB05
	LXI H, 16
	MOV A, M
	LXI H, 12
	CMP M
	JNZ LBB05
	LXI H, 17
	MOV A, M
	LXI H, 13
	CMP M
	JNZ LBB05
	LXI H, 18
	MOV A, M
	LXI H, 14
	CMP M
	JNZ LBB05
	JMP LBB02
LBB02:
	MVI B, 0
	JMP LBB06
LBB03:
	LXI H, 18
	MOV A, M
	ANI 128
	JZ LBB04
	JMP LBB07
LBB04:
	MVI B, 1
	JMP LBB08
LBB05:
	MVI B, 1
	JMP LBB06
LBB06:
	JMP LBB09
LBB07:
	MVI B, 0
	JMP LBB08
LBB08:
	JMP LBB09
LBB09:
	MOV A, B
	ORI 0
	JNZ LBB011
	JMP LBB010
LBB010:
	LXI H, 11
	DAD SP
	MOV B, M
	LXI H, 10
	DAD SP
	MOV C, M
	LXI H, 13
	DAD SP
	MOV M, B
	LXI H, 12
	DAD SP
	MOV M, C
	JMP LBB012
LBB011:
	MVI B, 0
	MVI C, 1
	LXI H, 11
	DAD SP
	MOV D, M
	LXI H, 10
	DAD SP
	MOV E, M
	MOV A, E
	SUB C
	MOV E, A
	MOV A, D
	SBB B
	MOV D, A
	LXI H, 1
	DAD SP
	MOV M, D
	LXI H, 0
	DAD SP
	MOV M, E
	CALL fibonacci
	LXI H, 7
	DAD SP
	MOV M, B
	LXI H, 6
	DAD SP
	MOV M, C
	MVI D, 0
	MVI E, 2
	LXI H, 11
	DAD SP
	MOV B, M
	LXI H, 10
	DAD SP
	MOV C, M
	MOV A, C
	SUB E
	MOV C, A
	MOV A, B
	SBB D
	MOV B, A
	LXI H, 1
	DAD SP
	MOV M, B
	LXI H, 0
	DAD SP
	MOV M, C
	LXI H, 7
	DAD SP
	MOV B, M
	LXI H, 6
	DAD SP
	MOV C, M
	LXI H, 14
	MVI M, 0
	LXI H, 13
	MVI M, 0
	LXI H, 12
	MOV M, B
	LXI H, 11
	MOV M, C
	LXI H, 11
	MOV A, M
	LXI H, 6
	DAD SP
	MOV M, A
	LXI H, 12
	MOV A, M
	LXI H, 7
	DAD SP
	MOV M, A
	LXI H, 13
	MOV A, M
	LXI H, 8
	DAD SP
	MOV M, A
	LXI H, 14
	MOV A, M
	LXI H, 9
	DAD SP
	MOV M, A
	CALL fibonacci
	LXI H, 14
	MVI M, 0
	LXI H, 13
	MVI M, 0
	LXI H, 12
	MOV M, B
	LXI H, 11
	MOV M, C
	LXI H, 6
	DAD SP
	MOV A, M
	LXI H, 15
	MOV M, A
	LXI H, 7
	DAD SP
	MOV A, M
	LXI H, 16
	MOV M, A
	LXI H, 8
	DAD SP
	MOV A, M
	LXI H, 17
	MOV M, A
	LXI H, 9
	DAD SP
	MOV A, M
	LXI H, 18
	MOV M, A
	LXI H, 11
	MOV A, M
	LXI H, 15
	ADD M
	LXI H, 15
	MOV M, A
	LXI H, 12
	MOV A, M
	LXI H, 16
	ADC M
	LXI H, 16
	MOV M, A
	LXI H, 13
	MOV A, M
	LXI H, 17
	ADC M
	LXI H, 17
	MOV M, A
	LXI H, 14
	MOV A, M
	LXI H, 18
	ADC M
	LXI H, 18
	MOV M, A
	LXI H, 15
	MOV C, M
	LXI H, 16
	MOV B, M
	LXI H, 13
	DAD SP
	MOV M, B
	LXI H, 12
	DAD SP
	MOV M, C
LBB012:
	LXI H, 13
	DAD SP
	MOV B, M
	LXI H, 12
	DAD SP
	MOV C, M
	LXI H, 11
	MOV M, C
	LXI H, 12
	MOV M, B
	MOV A, B
	ADI 128
	SBB A
	LXI H, 14
	MOV M, A
	LXI H, 13
	MOV M, A
	LXI H, 11
	MOV A, M
	LXI H, 16
	DAD SP
	MOV M, A
	LXI H, 12
	MOV A, M
	LXI H, 17
	DAD SP
	MOV M, A
	LXI H, 13
	MOV A, M
	LXI H, 18
	DAD SP
	MOV M, A
	LXI H, 14
	MOV A, M
	LXI H, 19
	DAD SP
	MOV M, A
	LXI H, 14
	DAD SP
	SPHL
	RET
```
