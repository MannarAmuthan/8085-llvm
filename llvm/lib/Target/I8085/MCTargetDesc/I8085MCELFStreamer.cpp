//===--------- I8085MCELFStreamer.cpp - I8085 subclass of MCELFStreamer -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a stub that parses a MCInst bundle and passes the
// instructions on to the real streamer.
//
//===----------------------------------------------------------------------===//
#include "MCTargetDesc/I8085MCELFStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbol.h"

#define DEBUG_TYPE "avrmcelfstreamer"

using namespace llvm;

void I8085MCELFStreamer::emitValueForModiferKind(
    const MCSymbol *Sym, unsigned SizeInBytes, SMLoc Loc,
    I8085MCExpr::VariantKind ModifierKind) {
  MCSymbolRefExpr::VariantKind Kind = MCSymbolRefExpr::VK_I8085_NONE;
  if (ModifierKind == I8085MCExpr::VK_I8085_None) {
    Kind = MCSymbolRefExpr::VK_I8085_DIFF8;
    if (SizeInBytes == SIZE_LONG)
      Kind = MCSymbolRefExpr::VK_I8085_DIFF32;
    else if (SizeInBytes == SIZE_WORD)
      Kind = MCSymbolRefExpr::VK_I8085_DIFF16;
  } else if (ModifierKind == I8085MCExpr::VK_I8085_LO8)
    Kind = MCSymbolRefExpr::VK_I8085_LO8;
  else if (ModifierKind == I8085MCExpr::VK_I8085_HI8)
    Kind = MCSymbolRefExpr::VK_I8085_HI8;
  else if (ModifierKind == I8085MCExpr::VK_I8085_HH8)
    Kind = MCSymbolRefExpr::VK_I8085_HLO8;
  MCELFStreamer::emitValue(MCSymbolRefExpr::create(Sym, Kind, getContext()),
                           SizeInBytes, Loc);
}

namespace llvm {
MCStreamer *createI8085ELFStreamer(Triple const &TT, MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> MAB,
                                 std::unique_ptr<MCObjectWriter> OW,
                                 std::unique_ptr<MCCodeEmitter> CE) {
  return new I8085MCELFStreamer(Context, std::move(MAB), std::move(OW),
                              std::move(CE));
}

} // end namespace llvm
