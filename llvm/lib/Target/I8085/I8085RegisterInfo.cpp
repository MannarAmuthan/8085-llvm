//===-- I8085RegisterInfo.cpp - I8085 Register Information --------------------===//
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

#include "I8085RegisterInfo.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/IR/Function.h"

#include "I8085.h"
#include "I8085InstrInfo.h"
#include "I8085MachineFunctionInfo.h"
#include "I8085TargetMachine.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"


#include <iostream>

#define GET_REGINFO_TARGET_DESC
#include "I8085GenRegisterInfo.inc"

namespace llvm {

I8085RegisterInfo::I8085RegisterInfo() : I8085GenRegisterInfo(0) {}

const uint16_t *
I8085RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const I8085MachineFunctionInfo *AFI = MF->getInfo<I8085MachineFunctionInfo>();
  const I8085Subtarget &STI = MF->getSubtarget<I8085Subtarget>();
    return CSR_Normal_SaveList;
}

const uint32_t *
I8085RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                      CallingConv::ID CC) const {
  const I8085Subtarget &STI = MF.getSubtarget<I8085Subtarget>();
  return CSR_Normal_RegMask;
}

BitVector I8085RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());


  Reserved.set(I8085::R0);
  Reserved.set(I8085::R1);
  Reserved.set(I8085::R1R0);

  // Reserve the stack pointer.
  Reserved.set(I8085::SPL);
  Reserved.set(I8085::SPH);
  Reserved.set(I8085::SP);


  if (MF.getSubtarget<I8085Subtarget>().hasTinyEncoding()) {
    // Reserve 8-bit registers R2~R15, Rtmp(R16) and Zero(R17).
    for (unsigned Reg = I8085::R2; Reg <= I8085::R17; Reg++)
      Reserved.set(Reg);
    // Reserve 16-bit registers R3R2~R18R17.
    for (unsigned Reg = I8085::R3R2; Reg <= I8085::R18R17; Reg++)
      Reserved.set(Reg);
  }

  Reserved.set(I8085::A);
  Reserved.set(I8085::H);
  Reserved.set(I8085::L);

  return Reserved;
}

const TargetRegisterClass *
I8085RegisterInfo::getLargestLegalSuperClass(const TargetRegisterClass *RC,
                                           const MachineFunction &MF) const {
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();


  if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    return &I8085::GR16RegClass;
  }

  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    return &I8085::GR8RegClass;
  }

  llvm_unreachable("Invalid register size");
}

/// Fold a frame offset shared between two add instructions into a single one.


void I8085RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {                                      
  assert(SPAdj == 0 && "Unexpected SPAdj value");

  MachineInstr &MI = *II;
  DebugLoc dl = MI.getDebugLoc();
  MachineBasicBlock &MBB = *MI.getParent();
  const MachineFunction &MF = *MBB.getParent();
  const I8085TargetMachine &TM = (const I8085TargetMachine &)MF.getTarget();
  const TargetInstrInfo &TII = *TM.getSubtargetImpl()->getInstrInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFI = TM.getSubtargetImpl()->getFrameLowering();
  const I8085Subtarget &STI = MF.getSubtarget<I8085Subtarget>();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int Offset = MFI.getObjectOffset(FrameIndex);

  // std::cout << "############" << "\n";
  // MI.dump();
  // std::cout << Offset << "\n";
  // std::cout << MFI.getStackSize() << "\n";
  // std::cout << TFI->getOffsetOfLocalArea() << "\n";
  // std::cout << FIOperandNum << "\n";
  // std::cout << MI.getOperand(FIOperandNum + 1).getImm() << "\n";


  // Add one to the offset because SP points to an empty slot.
  // Offset += MFI.getStackSize() - TFI->getOffsetOfLocalArea() + 1;

  Offset += MFI.getStackSize() - TFI->getOffsetOfLocalArea();
  // Fold incoming offset.
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  // std::cout << Offset << "\n";
  // std::cout << "############" << "\n";
  

  // If the offset is too big we have to adjust and restore the frame pointer
  // to materialize a valid load/store with displacement.
  //: TODO: consider using only one adiw/sbiw chain for more than one frame
  //: index
  
  MI.getOperand(FIOperandNum).ChangeToRegister(I8085::L, false);
  assert(isUInt<6>(Offset) && "Offset is out of range");
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

Register I8085RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  if (TFI->hasFP(MF)) {
    // The Y pointer register
    return I8085::L;
  }

  return I8085::SP;
}

const TargetRegisterClass *
I8085RegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                    unsigned Kind) const {
  return &I8085::PTRDISPREGSRegClass;
}

void I8085RegisterInfo::splitReg(Register Reg, Register &LoReg,
                               Register &HiReg) const {                        
  assert(I8085::DREGSRegClass.contains(Reg) && "can only split 16-bit registers");

  LoReg = getSubReg(Reg, I8085::sub_lo);
  HiReg = getSubReg(Reg, I8085::sub_hi);
}

bool I8085RegisterInfo::shouldCoalesce(
    MachineInstr *MI, const TargetRegisterClass *SrcRC, unsigned SubReg,
    const TargetRegisterClass *DstRC, unsigned DstSubReg,
    const TargetRegisterClass *NewRC, LiveIntervals &LIS) const {
  if (this->getRegClass(I8085::PTRDISPREGSRegClassID)->hasSubClassEq(NewRC)) {
    return false;
  }

  return TargetRegisterInfo::shouldCoalesce(MI, SrcRC, SubReg, DstRC, DstSubReg,
                                            NewRC, LIS);
}

} // end of namespace llvm
