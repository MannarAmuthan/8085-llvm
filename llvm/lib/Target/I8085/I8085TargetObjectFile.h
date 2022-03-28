//===-- I8085TargetObjectFile.h - I8085 Object Info -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_TARGET_OBJECT_FILE_H
#define LLVM_I8085_TARGET_OBJECT_FILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

/// Lowering for an I8085 ELF32 object file.
class I8085TargetObjectFile : public TargetLoweringObjectFileELF {
  typedef TargetLoweringObjectFileELF Base;

public:
  void Initialize(MCContext &ctx, const TargetMachine &TM) override;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;

private:
  MCSection *ProgmemDataSection;
  MCSection *Progmem1DataSection;
  MCSection *Progmem2DataSection;
  MCSection *Progmem3DataSection;
  MCSection *Progmem4DataSection;
  MCSection *Progmem5DataSection;
};

} // end namespace llvm

#endif // LLVM_I8085_TARGET_OBJECT_FILE_H
