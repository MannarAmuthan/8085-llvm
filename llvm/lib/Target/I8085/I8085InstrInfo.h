//===-- I8085InstrInfo.h - I8085 Instruction Information ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the I8085 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_INSTR_INFO_H
#define LLVM_I8085_INSTR_INFO_H

#include "llvm/CodeGen/TargetInstrInfo.h"

#include "I8085RegisterInfo.h"

#define GET_INSTRINFO_HEADER
#include "I8085GenInstrInfo.inc"
#undef GET_INSTRINFO_HEADER

namespace llvm {

namespace I8085CC {

/// I8085 specific condition codes.
/// These correspond to `I8085_*_COND` in `I8085InstrInfo.td`.
/// They must be kept in synch.
enum CondCodes {
  COND_EQ, //!< Equal
  COND_NE, //!< Not equal
  COND_GE, //!< Greater than or equal
  COND_LT, //!< Less than
  COND_SH, //!< Unsigned same or higher
  COND_LO, //!< Unsigned lower
  COND_MI, //!< Minus
  COND_PL, //!< Plus
  COND_INVALID
};

} // end of namespace I8085CC

namespace I8085II {

/// Specifies a target operand flag.
enum TOF {
  MO_NO_FLAG,

  /// On a symbol operand, this represents the lo part.
  MO_LO = (1 << 1),

  /// On a symbol operand, this represents the hi part.
  MO_HI = (1 << 2),

  /// On a symbol operand, this represents it has to be negated.
  MO_NEG = (1 << 3)
};

} // end of namespace I8085II

/// Utilities related to the I8085 instruction set.
class I8085InstrInfo : public I8085GenInstrInfo {
public:
  explicit I8085InstrInfo();

  const I8085RegisterInfo &getRegisterInfo() const { return RI; }
  // const MCInstrDesc &getBrCond(I8085CC::CondCodes CC) const;
  // I8085CC::CondCodes getCondFromBranchOpc(unsigned Opc) const;
  // I8085CC::CondCodes getOppositeCondition(I8085CC::CondCodes CC) const;
  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, Register SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI, Register VReg) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI, Register VReg) const override;
  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc,
                             int64_t BrOffset) const override;

private:
  const I8085RegisterInfo RI;
};

} // end namespace llvm

#endif // LLVM_I8085_INSTR_INFO_H
