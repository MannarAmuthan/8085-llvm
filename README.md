# Experimental LLVM Backend for Intel 8085 architecture

For more details, current status of this backend, Please refer this [target's readme](llvm/lib/Target/I8085).

Example : fibonacci.ll

```ll
define i16 @fibonacci(i16 %a) {
entry:
    %tmp1 = icmp sle i16 %a, 1
    br i1 %tmp1, label %done, label %recurse
    recurse:
        %tmp2 = sub i16 %a, 1
        %tmp3 = sub i16 %a, 2
        %tmp4 = call i16 @fibonacci(i16 %tmp2)
        %tmp5 = call i16 @fibonacci(i16 %tmp3)
        %tmp6 = add i16 %tmp4,%tmp5
        ret i16 %tmp6
    done:
        ret i16 %a
}
```

```assembly
fibonacci:
LBB00:
	LXI H, 65530
	DAD SP
	SPHL
	MVI D, 0
	MVI E, 2
	LXI H, 9
	DAD SP
	MOV B, M
	LXI H, 8
	DAD SP
	MOV C, M
	LXI H, 5
	DAD SP
	MOV M, B
	LXI H, 4
	DAD SP
	MOV M, C
	MOV A, B
	XRA D
	ANI 128
	JZ	LBB01
	JMP LBB04
LBB01:
	LXI H, 3
	DAD SP
	MOV M, D
	LXI H, 2
	DAD SP
	MOV M, E
	MOV D, B
	MOV E, C
	LXI H, 3
	DAD SP
	MOV B, M
	LXI H, 2
	DAD SP
	MOV C, M
	MOV A, E
	SUB C
	MOV E, A
	MOV A, D
	SBB B
	MOV D, A
	MVI D, 1
	JC	LBB03
	JMP LBB02
LBB02:
	MVI D, 0
	JMP LBB03
LBB03:
	LXI H, 5
	DAD SP
	MOV B, M
	LXI H, 4
	DAD SP
	MOV C, M
	JMP LBB08
LBB04:
	MOV A, B
	ANI 128
	JZ	LBB05
	JMP LBB06
LBB05:
	MVI D, 0
	JMP LBB07
LBB06:
	MVI D, 1
	JMP LBB07
LBB07:
	JMP LBB08
LBB08:
	MOV A, D
	ORI 0
	JNZ	LBB010
	JMP LBB09
LBB09:
	MVI D, 0
	MVI E, 1
	LXI H, 3
	DAD SP
	MOV M, D
	LXI H, 2
	DAD SP
	MOV M, E
	MOV D, B
	MOV E, C
	LXI H, 3
	DAD SP
	MOV B, M
	LXI H, 2
	DAD SP
	MOV C, M
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
	MVI B, 0
	MVI C, 2
	LXI H, 5
	DAD SP
	MOV D, M
	LXI H, 4
	DAD SP
	MOV E, M
	MOV A, E
	SUB C
	MOV E, A
	MOV A, D
	SBB B
	MOV D, A
	LXI H, 5
	DAD SP
	MOV M, D
	LXI H, 4
	DAD SP
	MOV M, E
	CALL fibonacci
	LXI H, 3
	DAD SP
	MOV M, B
	LXI H, 2
	DAD SP
	MOV M, C
	LXI H, 5
	DAD SP
	MOV B, M
	LXI H, 4
	DAD SP
	MOV C, M
	LXI H, 1
	DAD SP
	MOV M, B
	LXI H, 0
	DAD SP
	MOV M, C
	CALL fibonacci
	LXI H, 3
	DAD SP
	MOV D, M
	LXI H, 2
	DAD SP
	MOV E, M
	MOV A, E
	ADD C
	MOV E, A
	MOV A, D
	ADC B
	MOV D, A
	MOV B, D
	MOV C, E
LBB010:
	LXI H, 6
	DAD SP
	SPHL
	RET
```


## Build

```
cd llvm 
mkdir build
cmake -G "Ninja" -DLLVM_TARGETS_TO_BUILD="I8085" -DCMAKE_BUILD_TYPE="Debug" -DLLVM_ENABLE_ASSERTIONS=On ../
ninja
```

For using other build systems and methods, please refer official LLVM Build Docs.
