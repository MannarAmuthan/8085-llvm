//===-- I8085MCAsmInfo.cpp - I8085 asm properties -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the I8085MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "I8085MCAsmInfo.h"

#include "llvm/TargetParser/Triple.h"

namespace llvm {

I8085MCAsmInfo::I8085MCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
  CodePointerSize = 2;
  CalleeSaveStackSlotSize = 2;
  CommentString = ";";
  PrivateGlobalPrefix = ".L";
  PrivateLabelPrefix = "L";
  UsesELFSectionDirectiveForBSS = true;
  SupportsDebugInformation = true;
}

} // end of namespace llvm
