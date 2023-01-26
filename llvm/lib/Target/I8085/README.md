
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

Currently basic constructs such as , loading and storing variables, function call, branching statements, logical operations (and, or, xor) and two arithmetic operations such as addition and subtraction are implemented, for i8,i16,i32 integere types. Still far way to go.


## <a id="notes">**Some implementation note:**

1. Arguments are passed via stack, and return value is stored in register.
2. 8 bit integers are returned via A reg, 16 bit integers are returned via BC reg pair and 32 bit integers are returned via stack.


## <a id="correctness">**Correctness:**

The output assembly is tested in open source 8085 simulators and assemblers.  Codegen Tests are located [here](https://github.com/MannarAmuthan/8085-llvm/tree/main/llvm/test/CodeGen/I8085).

## <a id="disclaimer">**Disclaimer:**

This 8085 backend is purely experimental and never tested with original hardware and never used in productional use cases. 

## <a id="priority">**Next on priority:**
1. Implement shift operations for 16 bit integers.
2. Linker/Loader implementation.
3. Adding more arithmetic operations and floating point types.


## <a id="credits"> **Thanks and Credits:**

This backend is inspired by LLVM backend for AVR 8 bit microcontroller. Some of the functionalities here is referred from there. 