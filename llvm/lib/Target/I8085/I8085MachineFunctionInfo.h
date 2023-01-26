//===-- I8085MachineFuctionInfo.h - I8085 machine function info -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares I8085-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_MACHINE_FUNCTION_INFO_H
#define LLVM_I8085_MACHINE_FUNCTION_INFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// Contains I8085-specific information for each MachineFunction.
class I8085MachineFunctionInfo : public MachineFunctionInfo {
  /// Indicates if a register has been spilled by the register
  /// allocator.
  bool HasSpills;

  /// Indicates if there are any fixed size allocas present.
  /// Note that if there are only variable sized allocas this is set to false.
  bool HasAllocas;

  /// Indicates if arguments passed using the stack are being
  /// used inside the function.
  bool HasStackArgs;

  /// Whether or not the function is an interrupt handler.
  bool IsInterruptHandler;

  /// Whether or not the function is an non-blocking interrupt handler.
  bool IsSignalHandler;

  /// Size of the callee-saved register portion of the
  /// stack frame in bytes.
  unsigned CalleeSavedFrameSize;

  /// FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

  public:
  I8085MachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : HasSpills(false), HasAllocas(false), HasStackArgs(false),
        CalleeSavedFrameSize(0), VarArgsFrameIndex(0) {
    CallingConv::ID CallConv = F.getCallingConv();

    this->IsInterruptHandler =
        CallConv == CallingConv::I8085_INTR || F.hasFnAttribute("interrupt");
    this->IsSignalHandler =
        CallConv == CallingConv::I8085_SIGNAL || F.hasFnAttribute("signal");
  }

  bool getHasSpills() const { return HasSpills; }
  void setHasSpills(bool B) { HasSpills = B; }

  bool getHasAllocas() const { return HasAllocas; }
  void setHasAllocas(bool B) { HasAllocas = B; }

  bool getHasStackArgs() const { return HasStackArgs; }
  void setHasStackArgs(bool B) { HasStackArgs = B; }

  /// Checks if the function is some form of interrupt service routine.
  bool isInterruptOrSignalHandler() const {
    return isInterruptHandler() || isSignalHandler();
  }

  bool isInterruptHandler() const { return IsInterruptHandler; }
  bool isSignalHandler() const { return IsSignalHandler; }

  unsigned getCalleeSavedFrameSize() const { return CalleeSavedFrameSize; }
  void setCalleeSavedFrameSize(unsigned Bytes) { CalleeSavedFrameSize = Bytes; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Idx) { VarArgsFrameIndex = Idx; }
};

} // namespace llvm

#endif // LLVM_I8085_MACHINE_FUNCTION_INFO_H
