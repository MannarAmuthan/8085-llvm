//===-- I8085MCCodeEmitter.h - Convert I8085 Code to Machine Code -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the I8085MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_I8085_CODE_EMITTER_H
#define LLVM_I8085_CODE_EMITTER_H

#include "I8085FixupKinds.h"

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/Support/DataTypes.h"

#define GET_INSTRINFO_OPERAND_TYPES_ENUM
#include "I8085GenInstrInfo.inc"

namespace llvm {

class MCContext;
class MCExpr;
class MCFixup;
class MCInst;
class MCInstrInfo;
class MCOperand;
class MCSubtargetInfo;
class raw_ostream;

/// Writes I8085 machine code to a stream.
class I8085MCCodeEmitter : public MCCodeEmitter {
public:
  I8085MCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
      : MCII(MCII), Ctx(Ctx) {}

private:
  
  template <I8085::Fixups Fixup, unsigned Offset>
  unsigned encodeCallTarget(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;

  template <I8085::Fixups Fixup, unsigned Offset>
  unsigned encodeImm(const MCInst &MI, unsigned OpNo,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const    ;                     

  /// TableGen'ed function to get the binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getExprOpValue(const MCExpr *Expr, SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  /// Returns the binary encoding of operand.
  ///
  /// If the machine operand requires relocation, the relocation is recorded
  /// and zero is returned.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  void emitInstruction(uint64_t Val, unsigned Size, const MCSubtargetInfo &STI,
                       raw_ostream &OS) const;

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  I8085MCCodeEmitter(const I8085MCCodeEmitter &) = delete;
  void operator=(const I8085MCCodeEmitter &) = delete;

  const MCInstrInfo &MCII;
  MCContext &Ctx;
};

} // namespace llvm

#endif // LLVM_I8085_CODE_EMITTER_H
