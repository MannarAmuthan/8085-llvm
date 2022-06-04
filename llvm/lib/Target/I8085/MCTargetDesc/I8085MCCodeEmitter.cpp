//===-- I8085MCCodeEmitter.cpp - Convert I8085 Code to Machine Code -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the I8085MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "I8085MCCodeEmitter.h"

#include "MCTargetDesc/I8085MCExpr.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"


// Added for Debuffing, needs to removed
#include <iostream>

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "I8085GenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

namespace llvm {

template <I8085::Fixups Fixup, unsigned Offset>
unsigned I8085MCCodeEmitter::encodeCallTarget(const MCInst &MI, unsigned OpNo,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  auto MO = MI.getOperand(OpNo);

  if (MO.isExpr()) {
    MCFixupKind FixupKind = static_cast<MCFixupKind>(Fixup);
    Fixups.push_back(MCFixup::create(Offset, MO.getExpr(), FixupKind, MI.getLoc()));
    return 0;
  }

  assert(MO.isImm());

  auto Target = MO.getImm();
  I8085::fixups::adjustBranchTarget(Target);
  return Target;
}

template <I8085::Fixups Fixup, unsigned Offset>
unsigned I8085MCCodeEmitter::encodeImm(const MCInst &MI, unsigned OpNo,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  auto MO = MI.getOperand(OpNo);

  if (MO.isExpr()) {
    if (isa<I8085MCExpr>(MO.getExpr())) {
      // If the expression is already an I8085MCExpr (i.e. a lo8(symbol),
      // we shouldn't perform any more fixups. Without this check, we would
      // instead create a fixup to the symbol named 'lo8(symbol)' which
      // is not correct.
      return getExprOpValue(MO.getExpr(), Fixups, STI);
    }

    MCFixupKind FixupKind = static_cast<MCFixupKind>(Fixup);
    Fixups.push_back(
        MCFixup::create(Offset, MO.getExpr(), FixupKind, MI.getLoc()));
    return 0;
  }

  assert(MO.isImm());
  return MO.getImm();
}

unsigned I8085MCCodeEmitter::getExprOpValue(const MCExpr *Expr,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {

  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr *>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  if (Kind == MCExpr::Target) {
    I8085MCExpr const *I8085Expr = cast<I8085MCExpr>(Expr);
    int64_t Result;
    if (I8085Expr->evaluateAsConstant(Result)) {
      return Result;
    }

    MCFixupKind FixupKind = static_cast<MCFixupKind>(I8085Expr->getFixupKind());
    Fixups.push_back(MCFixup::create(0, I8085Expr, FixupKind));
    return 0;
  }

  assert(Kind == MCExpr::SymbolRef);
  return 0;
}

unsigned I8085MCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                             const MCOperand &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  if (MO.isReg()){
    // std::cout << "Register encode val: "<< Ctx.getRegisterInfo()->getEncodingValue(MO.getReg()) << "\n";
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());}
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  if (MO.isDFPImm())
    return static_cast<unsigned>(bit_cast<double>(MO.getDFPImm()));

  // MO must be an Expr.
  assert(MO.isExpr());

  return getExprOpValue(MO.getExpr(), Fixups, STI);
}

void I8085MCCodeEmitter::emitInstruction(uint64_t Val, unsigned Size,
                                       const MCSubtargetInfo &STI,
                                       raw_ostream &OS) const {

  if(Size == 1){
    uint8_t val = Val & 0xff;
    // std::cout << "Writing Byte.. " << val << "\n";
    OS << (char)val;
  }

  if(Size == 2){
    uint16_t val = Val;
    // std::cout << "Writing Word .. " << val << "\n";
    uint8_t opcode = (val & 0xFF00) >> 8;
    uint8_t operand = (val & 0x00FF);
    OS << (char)opcode;
    OS << (char)operand;
  }
  
  if(Size == 3){
    uint32_t val = Val;
    uint8_t opcode = (val & 0x00FF0000) >> 16;
    uint8_t operandHigh = (val & 0x0000FF00) >> 8;
    uint8_t operandLow = (val & 0x000000FF);
    OS << (char)opcode;
    OS << (char)operandLow;
    OS << (char)operandHigh;
  }
}

void I8085MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction
  unsigned Size = Desc.getSize();

  if(Size==0){
    MI.dump();
  }

  assert(Size > 0 && "Instruction size cannot be zero");

  uint64_t BinaryOpCode = getBinaryCodeForInstr(MI, Fixups, STI);
  emitInstruction(BinaryOpCode, Size, STI, OS);
}

MCCodeEmitter *createI8085MCCodeEmitter(const MCInstrInfo &MCII,
                                      MCContext &Ctx) {
  return new I8085MCCodeEmitter(MCII, Ctx);
}

#include "I8085GenMCCodeEmitter.inc"

} // end of namespace llvm
