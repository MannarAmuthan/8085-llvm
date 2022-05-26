//===-- I8085Subtarget.cpp - I8085 Subtarget Information ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the I8085 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "I8085Subtarget.h"

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/TargetRegistry.h"

#include "I8085.h"
#include "I8085TargetMachine.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

#define DEBUG_TYPE "i8085-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "I8085GenSubtargetInfo.inc"

namespace llvm {

I8085Subtarget::I8085Subtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const I8085TargetMachine &TM)
    : I8085GenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), ELFArch(0),

      TLInfo(TM, initializeSubtargetDependencies(CPU, FS, TM)) {
            // Parse features string.
            ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
      }

I8085Subtarget &
I8085Subtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                              const TargetMachine &TM) {
  // Parse features string.
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
  return *this;
}

} // end of namespace llvm
