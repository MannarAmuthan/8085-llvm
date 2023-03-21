//===-- I8085AsmBackend.h - I8085 Asm Backend  --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// \file The I8085 assembly backend implementation.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_I8085_ASM_BACKEND_H
#define LLVM_I8085_ASM_BACKEND_H

#include "MCTargetDesc/I8085FixupKinds.h"

#include "llvm/TargetParser/Triple.h"
#include "llvm/MC/MCAsmBackend.h"

namespace llvm {

class MCAssembler;
class MCContext;
struct MCFixupKindInfo;

/// Utilities for manipulating generated I8085 machine code.
class I8085AsmBackend : public MCAsmBackend {
public:
  I8085AsmBackend(Triple::OSType OSType)
      : MCAsmBackend(support::little), OSType(OSType) {}

  void adjustFixupValue(const MCFixup &Fixup, const MCValue &Target,
                        uint64_t &Value, MCContext *Ctx = nullptr) const;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  unsigned getNumFixupKinds() const override {
    return I8085::NumTargetFixupKinds;
  }

  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    llvm_unreachable("RelaxInstruction() unimplemented");
    return false;
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  bool shouldForceRelocation(const MCAssembler &Asm, const MCFixup &Fixup,
                             const MCValue &Target) override;

private:
  Triple::OSType OSType;
};

} // end namespace llvm

#endif // LLVM_I8085_ASM_BACKEND_H
