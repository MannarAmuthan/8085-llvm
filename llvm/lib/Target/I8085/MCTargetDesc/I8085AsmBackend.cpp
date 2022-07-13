//===-- I8085AsmBackend.cpp - I8085 Asm Backend  ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the I8085AsmBackend class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/I8085AsmBackend.h"
#include "MCTargetDesc/I8085FixupKinds.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>

// FIXME: we should be doing checks to make sure asm operands
// are not out of bounds.

namespace adjust {

using namespace llvm;

} // namespace adjust

namespace llvm {

// Prepare value for the target space for it
void I8085AsmBackend::adjustFixupValue(const MCFixup &Fixup,
                                     const MCValue &Target, uint64_t &Value,
                                     MCContext *Ctx) const {
  // The size of the fixup in bits.
  uint64_t Size = I8085AsmBackend::getFixupKindInfo(Fixup.getKind()).TargetSize;

  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  default:
    llvm_unreachable("unhandled fixup");
  
  case I8085::fixup_16:
    Value &= 0xffff;
    break;

  }
}

std::unique_ptr<MCObjectTargetWriter>
I8085AsmBackend::createObjectTargetWriter() const {
  return createI8085ELFObjectWriter(MCELFObjectTargetWriter::getOSABI(OSType));
}

void I8085AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target,
                               MutableArrayRef<char> Data, uint64_t Value,
                               bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  adjustFixupValue(Fixup, Target, Value, &Asm.getContext());
  if (Value == 0){
    return;} // Doesn't change encoding.

  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

  // The number of bits in the fixup mask
  auto NumBits = Info.TargetSize + Info.TargetOffset;
  auto NumBytes = (NumBits / 8) + ((NumBits % 8) == 0 ? 0 : 1);

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  unsigned Offset = Fixup.getOffset();
  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i < NumBytes; ++i) {
    uint8_t mask = (((Value >> (i * 8)) & 0xff));
    Data[Offset + i] |= mask;
  }
}

MCFixupKindInfo const &I8085AsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  // NOTE: Many I8085 fixups work on sets of non-contignous bits. We work around
  // this by saying that the fixup is the size of the entire instruction.
  const static MCFixupKindInfo Infos[I8085::NumTargetFixupKinds] = {
      {"fixup_16", 0, 16, 0},
  };

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");

  return Infos[Kind - FirstTargetFixupKind];
}

bool I8085AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                 const MCSubtargetInfo *STI) const {
  // If the count is not 2-byte aligned, we must be writing data into the text
  // section (otherwise we have unaligned instructions, and thus have far
  // bigger problems), so just write zeros instead.
  assert((Count % 2) == 0 && "NOP instructions must be 2 bytes");

  OS.write_zeros(Count);
  return true;
}

bool I8085AsmBackend::shouldForceRelocation(const MCAssembler &Asm,
                                          const MCFixup &Fixup,
                                          const MCValue &Target) {
  switch ((unsigned)Fixup.getKind()) {
  default:
    return false;
  case I8085::fixup_16:
    return true;
  }
}

MCAsmBackend *createI8085AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const llvm::MCTargetOptions &TO) {
  return new I8085AsmBackend(STI.getTargetTriple().getOS());
}

} // end of namespace llvm
