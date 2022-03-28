//===-- I8085MCAsmInfo.h - I8085 asm properties ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the I8085MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_ASM_INFO_H
#define LLVM_I8085_ASM_INFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {

class Triple;

/// Specifies the format of I8085 assembly files.
class I8085MCAsmInfo : public MCAsmInfo {
public:
  explicit I8085MCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
};

} // end namespace llvm

#endif // LLVM_I8085_ASM_INFO_H
