//===-- I8085TargetInfo.cpp - I8085 Target Implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/I8085TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
namespace llvm {
Target &getTheI8085Target() {
  static Target TheI8085Target;
  return TheI8085Target;
}
} // namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeI8085TargetInfo() {
  llvm::RegisterTarget<llvm::Triple::i8085> X(llvm::getTheI8085Target(), "i8085",
                                            "I8085", "I8085");
}
