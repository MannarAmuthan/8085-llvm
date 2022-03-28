//===-- I8085MCExpr.cpp - I8085 specific MC expression classes ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "I8085MCExpr.h"

#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCValue.h"

namespace llvm {

namespace {

const struct ModifierEntry {
  const char *const Spelling;
  I8085MCExpr::VariantKind VariantKind;
} ModifierNames[] = {
    {"lo8", I8085MCExpr::VK_I8085_LO8},       {"hi8", I8085MCExpr::VK_I8085_HI8},
    {"hh8", I8085MCExpr::VK_I8085_HH8}, // synonym with hlo8
    {"hlo8", I8085MCExpr::VK_I8085_HH8},      {"hhi8", I8085MCExpr::VK_I8085_HHI8},

    {"pm", I8085MCExpr::VK_I8085_PM},         {"pm_lo8", I8085MCExpr::VK_I8085_PM_LO8},
    {"pm_hi8", I8085MCExpr::VK_I8085_PM_HI8}, {"pm_hh8", I8085MCExpr::VK_I8085_PM_HH8},

    {"lo8_gs", I8085MCExpr::VK_I8085_LO8_GS}, {"hi8_gs", I8085MCExpr::VK_I8085_HI8_GS},
    {"gs", I8085MCExpr::VK_I8085_GS},
};

} // end of anonymous namespace

const I8085MCExpr *I8085MCExpr::create(VariantKind Kind, const MCExpr *Expr,
                                   bool Negated, MCContext &Ctx) {
  return new (Ctx) I8085MCExpr(Kind, Expr, Negated);
}

void I8085MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  assert(Kind != VK_I8085_None);

  if (isNegated())
    OS << '-';

  OS << getName() << '(';
  getSubExpr()->print(OS, MAI);
  OS << ')';
}

bool I8085MCExpr::evaluateAsConstant(int64_t &Result) const {
  MCValue Value;

  bool isRelocatable =
      getSubExpr()->evaluateAsRelocatable(Value, nullptr, nullptr);

  if (!isRelocatable)
    return false;

  if (Value.isAbsolute()) {
    Result = evaluateAsInt64(Value.getConstant());
    return true;
  }

  return false;
}

bool I8085MCExpr::evaluateAsRelocatableImpl(MCValue &Result,
                                          const MCAsmLayout *Layout,
                                          const MCFixup *Fixup) const {
  MCValue Value;
  bool isRelocatable = SubExpr->evaluateAsRelocatable(Value, Layout, Fixup);

  if (!isRelocatable)
    return false;

  if (Value.isAbsolute()) {
    Result = MCValue::get(evaluateAsInt64(Value.getConstant()));
  } else {
    if (!Layout)
      return false;

    MCContext &Context = Layout->getAssembler().getContext();
    const MCSymbolRefExpr *Sym = Value.getSymA();
    MCSymbolRefExpr::VariantKind Modifier = Sym->getKind();
    if (Modifier != MCSymbolRefExpr::VK_None)
      return false;
    if (Kind == VK_I8085_PM) {
      Modifier = MCSymbolRefExpr::VK_I8085_PM;
    }

    Sym = MCSymbolRefExpr::create(&Sym->getSymbol(), Modifier, Context);
    Result = MCValue::get(Sym, Value.getSymB(), Value.getConstant());
  }

  return true;
}

int64_t I8085MCExpr::evaluateAsInt64(int64_t Value) const {
  if (Negated)
    Value *= -1;

  switch (Kind) {
  case I8085MCExpr::VK_I8085_LO8:
    Value &= 0xff;
    break;
  case I8085MCExpr::VK_I8085_HI8:
    Value &= 0xff00;
    Value >>= 8;
    break;
  case I8085MCExpr::VK_I8085_HH8:
    Value &= 0xff0000;
    Value >>= 16;
    break;
  case I8085MCExpr::VK_I8085_HHI8:
    Value &= 0xff000000;
    Value >>= 24;
    break;
  case I8085MCExpr::VK_I8085_PM_LO8:
  case I8085MCExpr::VK_I8085_LO8_GS:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    Value &= 0xff;
    break;
  case I8085MCExpr::VK_I8085_PM_HI8:
  case I8085MCExpr::VK_I8085_HI8_GS:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    Value &= 0xff00;
    Value >>= 8;
    break;
  case I8085MCExpr::VK_I8085_PM_HH8:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    Value &= 0xff0000;
    Value >>= 16;
    break;
  case I8085MCExpr::VK_I8085_PM:
  case I8085MCExpr::VK_I8085_GS:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    break;

  case I8085MCExpr::VK_I8085_None:
    llvm_unreachable("Uninitialized expression.");
  }
  return static_cast<uint64_t>(Value) & 0xff;
}

I8085::Fixups I8085MCExpr::getFixupKind() const {
  I8085::Fixups Kind = I8085::Fixups::LastTargetFixupKind;

  switch (getKind()) {
  case VK_I8085_LO8:
    Kind = isNegated() ? I8085::fixup_lo8_ldi_neg : I8085::fixup_lo8_ldi;
    break;
  case VK_I8085_HI8:
    Kind = isNegated() ? I8085::fixup_hi8_ldi_neg : I8085::fixup_hi8_ldi;
    break;
  case VK_I8085_HH8:
    Kind = isNegated() ? I8085::fixup_hh8_ldi_neg : I8085::fixup_hh8_ldi;
    break;
  case VK_I8085_HHI8:
    Kind = isNegated() ? I8085::fixup_ms8_ldi_neg : I8085::fixup_ms8_ldi;
    break;

  case VK_I8085_PM_LO8:
    Kind = isNegated() ? I8085::fixup_lo8_ldi_pm_neg : I8085::fixup_lo8_ldi_pm;
    break;
  case VK_I8085_PM_HI8:
    Kind = isNegated() ? I8085::fixup_hi8_ldi_pm_neg : I8085::fixup_hi8_ldi_pm;
    break;
  case VK_I8085_PM_HH8:
    Kind = isNegated() ? I8085::fixup_hh8_ldi_pm_neg : I8085::fixup_hh8_ldi_pm;
    break;
  case VK_I8085_PM:
  case VK_I8085_GS:
    Kind = I8085::fixup_16_pm;
    break;
  case VK_I8085_LO8_GS:
    Kind = I8085::fixup_lo8_ldi_gs;
    break;
  case VK_I8085_HI8_GS:
    Kind = I8085::fixup_hi8_ldi_gs;
    break;

  case VK_I8085_None:
    llvm_unreachable("Uninitialized expression");
  }

  return Kind;
}

void I8085MCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

const char *I8085MCExpr::getName() const {
  const auto &Modifier =
      llvm::find_if(ModifierNames, [this](ModifierEntry const &Mod) {
        return Mod.VariantKind == Kind;
      });

  if (Modifier != std::end(ModifierNames)) {
    return Modifier->Spelling;
  }
  return nullptr;
}

I8085MCExpr::VariantKind I8085MCExpr::getKindByName(StringRef Name) {
  const auto &Modifier =
      llvm::find_if(ModifierNames, [&Name](ModifierEntry const &Mod) {
        return Mod.Spelling == Name;
      });

  if (Modifier != std::end(ModifierNames)) {
    return Modifier->VariantKind;
  }
  return VK_I8085_None;
}

} // end of namespace llvm
