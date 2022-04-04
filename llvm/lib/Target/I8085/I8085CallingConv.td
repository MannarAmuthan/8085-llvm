//===-- I8085CallingConv.td - Calling Conventions for I8085 ----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for I8085 architecture.
// Normal functions use a special calling convention, solved in code.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// I8085 Return Value Calling Convention
//===----------------------------------------------------------------------===//

// Special return value calling convention for runtime functions.
def RetCC_I8085_BUILTIN : CallingConv<[

  CCIfType<[i8], CCAssignToReg<[A]>>,
  
  // CCIfType<[i16], CCAssignToStack<2, 1>>
  // CCIfType<[i8], CCAssignToReg<[R24, R25]>>,
  // CCIfType<[i16], CCAssignToReg<[R23R22, R25R24]>>
]>;

//===----------------------------------------------------------------------===//
// I8085 Argument Calling Conventions
//===----------------------------------------------------------------------===//

// The calling conventions are implemented in custom C++ code

// Calling convention for variadic functions.
def ArgCC_I8085_Vararg : CallingConv<[
  // i8 are always passed through the stack with a byte slot and byte alignment.
  CCIfType<[i8], CCAssignToStack<1, 1>>,
  // i16 are always passed through the stack with an alignment of 1.
  CCAssignToStack<2, 1>
]>;

//===----------------------------------------------------------------------===//
// Callee-saved register lists.
//===----------------------------------------------------------------------===//

def CSR_Normal : CalleeSavedRegs<(add D, E, (sequence "R%u", 17, 2))>;
