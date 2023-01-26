//===-- I8085RegisterInfo.h - I8085 Register Information Impl -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the I8085 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_I8085_REGISTER_INFO_H
#define LLVM_I8085_REGISTER_INFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "I8085GenRegisterInfo.inc"

namespace llvm {

/// Utilities relating to I8085 registers.
class I8085RegisterInfo : public I8085GenRegisterInfo {
public:
  I8085RegisterInfo();

public:
  const uint16_t *
  getCalleeSavedRegs(const MachineFunction *MF = nullptr) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  const TargetRegisterClass *
  getLargestLegalSuperClass(const TargetRegisterClass *RC,
                            const MachineFunction &MF) const override;

  /// Stack Frame Processing Methods
  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  // const TargetRegisterClass *
  // getPointerRegClass(const MachineFunction &MF,
  //                    unsigned Kind = 0) const override;

  /// Splits a 16-bit `DREGS` register into the lo/hi register pair.
  /// \param Reg A 16-bit register to split.
  void splitReg(Register Reg, Register &LoReg, Register &HiReg) const;

  bool shouldCoalesce(MachineInstr *MI, const TargetRegisterClass *SrcRC,
                      unsigned SubReg, const TargetRegisterClass *DstRC,
                      unsigned DstSubReg, const TargetRegisterClass *NewRC,
                      LiveIntervals &LIS) const override;
};

} // end namespace llvm

#endif // LLVM_I8085_REGISTER_INFO_H
