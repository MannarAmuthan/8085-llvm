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
  case FK_Data_1:
    switch (Modifier) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case MCSymbolRefExpr::VK_None:
      return ELF::R_I8085_8;
    case MCSymbolRefExpr::VK_I8085_DIFF8:
      return ELF::R_I8085_DIFF8;
    case MCSymbolRefExpr::VK_I8085_LO8:
      return ELF::R_I8085_8_LO8;
    case MCSymbolRefExpr::VK_I8085_HI8:
      return ELF::R_I8085_8_HI8;
    case MCSymbolRefExpr::VK_I8085_HLO8:
      return ELF::R_I8085_8_HLO8;
    }
  case FK_Data_4:
    switch (Modifier) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case MCSymbolRefExpr::VK_None:
      return ELF::R_I8085_32;
    case MCSymbolRefExpr::VK_I8085_DIFF32:
      return ELF::R_I8085_DIFF32;
    }
  case FK_Data_2:
    switch (Modifier) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case MCSymbolRefExpr::VK_None:
      return ELF::R_I8085_16;
    case MCSymbolRefExpr::VK_I8085_NONE:
    case MCSymbolRefExpr::VK_I8085_PM:
      return ELF::R_I8085_16_PM;
    case MCSymbolRefExpr::VK_I8085_DIFF16:
      return ELF::R_I8085_DIFF16;
    }
  case I8085::fixup_32:
    return ELF::R_I8085_32;
  case I8085::fixup_7_pcrel:
    return ELF::R_I8085_7_PCREL;
  case I8085::fixup_13_pcrel:
    return ELF::R_I8085_13_PCREL;
  case I8085::fixup_16:
    return ELF::R_I8085_16;
  case I8085::fixup_16_pm:
    return ELF::R_I8085_16_PM;
  case I8085::fixup_lo8_ldi:
    return ELF::R_I8085_LO8_LDI;
  case I8085::fixup_hi8_ldi:
    return ELF::R_I8085_HI8_LDI;
  case I8085::fixup_hh8_ldi:
    return ELF::R_I8085_HH8_LDI;
  case I8085::fixup_lo8_ldi_neg:
    return ELF::R_I8085_LO8_LDI_NEG;
  case I8085::fixup_hi8_ldi_neg:
    return ELF::R_I8085_HI8_LDI_NEG;
  case I8085::fixup_hh8_ldi_neg:
    return ELF::R_I8085_HH8_LDI_NEG;
  case I8085::fixup_lo8_ldi_pm:
    return ELF::R_I8085_LO8_LDI_PM;
  case I8085::fixup_hi8_ldi_pm:
    return ELF::R_I8085_HI8_LDI_PM;
  case I8085::fixup_hh8_ldi_pm:
    return ELF::R_I8085_HH8_LDI_PM;
  case I8085::fixup_lo8_ldi_pm_neg:
    return ELF::R_I8085_LO8_LDI_PM_NEG;
  case I8085::fixup_hi8_ldi_pm_neg:
    return ELF::R_I8085_HI8_LDI_PM_NEG;
  case I8085::fixup_hh8_ldi_pm_neg:
    return ELF::R_I8085_HH8_LDI_PM_NEG;
  case I8085::fixup_call:
    return ELF::R_I8085_CALL;
  case I8085::fixup_ldi:
    return ELF::R_I8085_LDI;
  case I8085::fixup_6:
    return ELF::R_I8085_6;
  case I8085::fixup_6_adiw:
    return ELF::R_I8085_6_ADIW;
  case I8085::fixup_ms8_ldi:
    return ELF::R_I8085_MS8_LDI;
  case I8085::fixup_ms8_ldi_neg:
    return ELF::R_I8085_MS8_LDI_NEG;
  case I8085::fixup_lo8_ldi_gs:
    return ELF::R_I8085_LO8_LDI_GS;
  case I8085::fixup_hi8_ldi_gs:
    return ELF::R_I8085_HI8_LDI_GS;
  case I8085::fixup_8:
    return ELF::R_I8085_8;
  case I8085::fixup_8_lo8:
    return ELF::R_I8085_8_LO8;
  case I8085::fixup_8_hi8:
    return ELF::R_I8085_8_HI8;
  case I8085::fixup_8_hlo8:
    return ELF::R_I8085_8_HLO8;
  case I8085::fixup_diff8:
    return ELF::R_I8085_DIFF8;
  case I8085::fixup_diff16:
    return ELF::R_I8085_DIFF16;
  case I8085::fixup_diff32:
    return ELF::R_I8085_DIFF32;
  case I8085::fixup_lds_sts_16:
    return ELF::R_I8085_LDS_STS_16;
  case I8085::fixup_port6:
    return ELF::R_I8085_PORT6;
  case I8085::fixup_port5:
    return ELF::R_I8085_PORT5;
  default:
    llvm_unreachable("invalid fixup kind!");
  }
}

std::unique_ptr<MCObjectTargetWriter> createI8085ELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<I8085ELFObjectWriter>(OSABI);
}

} // end of namespace llvm
