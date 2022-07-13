//===-- I8085ELFObjectWriter.cpp - I8085 ELF Writer ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/I8085FixupKinds.h"
#include "MCTargetDesc/I8085MCExpr.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

/// Writes I8085 machine code into an ELF32 object file.
class I8085ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  I8085ELFObjectWriter(uint8_t OSABI);

  virtual ~I8085ELFObjectWriter() = default;

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};

I8085ELFObjectWriter::I8085ELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_I8085, true) {}

unsigned I8085ELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  MCSymbolRefExpr::VariantKind Modifier = Target.getAccessVariant();
  switch ((unsigned)Fixup.getKind()) {

  case I8085::fixup_16:
    return ELF::R_I8085_16;

  default:
    llvm_unreachable("invalid fixup kind!");
  }
}

std::unique_ptr<MCObjectTargetWriter> createI8085ELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<I8085ELFObjectWriter>(OSABI);
}

} // end of namespace llvm
