//===-- I8085InstrInfo.cpp - I8085 Instruction Information --------------------===//
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

#include "I8085InstrInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

#include "I8085.h"
#include "I8085MachineFunctionInfo.h"
#include "I8085RegisterInfo.h"
#include "I8085TargetMachine.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

#include <iostream>

#define GET_INSTRINFO_CTOR_DTOR
#include "I8085GenInstrInfo.inc"

namespace llvm {

I8085InstrInfo::I8085InstrInfo()
    : I8085GenInstrInfo(I8085::ADJCALLSTACKDOWN, I8085::ADJCALLSTACKUP), RI() {}

void I8085InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MI,
                               const DebugLoc &DL, MCRegister DestReg,
                               MCRegister SrcReg, bool KillSrc) const {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  unsigned Opc;


  if (I8085::GR8RegClass.contains(DestReg, SrcReg)) {
      Opc = I8085::MOV;

      BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if(I8085::GR16RegClass.contains(DestReg, SrcReg)) {

  unsigned destLow,destHigh;
  if(DestReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(DestReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  unsigned srcLow,srcHigh;
  if(SrcReg==I8085::BC){  srcLow=I8085::C;  srcHigh=I8085::B; }
  if(SrcReg==I8085::DE){  srcLow=I8085::E;  srcHigh=I8085::D; }

  Opc = I8085::MOV;
  BuildMI(MBB, MI, DL, get(Opc), destHigh)
        .addReg(srcHigh, getKillRegState(KillSrc));
  BuildMI(MBB, MI, DL, get(Opc), destLow)
        .addReg(srcLow, getKillRegState(KillSrc));    
  }
  else{
  Opc = I8085::MOV_32;
  BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
}

unsigned I8085InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case I8085::LOAD_8_WITH_ADDR:
  case I8085::LOAD_16_WITH_ADDR:
  case I8085::LOAD_32_WITH_ADDR: { 
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
    break;
  }
  default:
    break;
  }

  return 0;
}

unsigned I8085InstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                          int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case I8085::STORE_8:
  case I8085::STORE_16: 
  case I8085::STORE_32: {
    if (MI.getOperand(0).isFI() && MI.getOperand(1).isImm() &&
        MI.getOperand(1).getImm() == 0) {
      FrameIndex = MI.getOperand(0).getIndex();
      return MI.getOperand(2).getReg();
    }
    break;
  }
  default:
    break;
  }

  return 0;
}

void I8085InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       Register SrcReg, bool isKill,
                                       int FrameIndex,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI,
                                       Register VReg) const {
  MachineFunction &MF = *MBB.getParent();
  I8085MachineFunctionInfo *AFI = MF.getInfo<I8085MachineFunctionInfo>();

  AFI->setHasSpills(true);

  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  const MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  unsigned Opcode = 0;
  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    Opcode = I8085::STORE_8;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = I8085::STORE_16;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i32)) {
    Opcode = I8085::STORE_32;
  } else {
    llvm_unreachable("Cannot store this register into a stack slot!");
  }

  BuildMI(MBB, MI, DL, get(Opcode))
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addReg(SrcReg, getKillRegState(isKill))
      .addMemOperand(MMO);
}

void I8085InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register DestReg, int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI,
                                        Register VReg) const {
  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex),
      MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
      MFI.getObjectAlign(FrameIndex));

  unsigned Opcode = 0;
  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    Opcode = I8085::LOAD_8_WITH_ADDR;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = I8085::LOAD_16_WITH_ADDR;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i32)) {
    Opcode = I8085::LOAD_32_WITH_ADDR;
  } else {
    llvm_unreachable("Cannot load this register from a stack slot!");
  }

  BuildMI(MBB, MI, DL, get(Opcode), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO);
}


unsigned I8085InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  unsigned Opcode = MI.getOpcode();

  switch (Opcode) {
  // A regular instruction
  default: {
    const MCInstrDesc &Desc = get(Opcode);
    return Desc.getSize();
  }
  case TargetOpcode::EH_LABEL:
  case TargetOpcode::IMPLICIT_DEF:
  case TargetOpcode::KILL:
  case TargetOpcode::DBG_VALUE:
    return 0;
  }
}

MachineBasicBlock *I8085InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected branch opcode!");
  case I8085::JMP:
  case I8085::JZ:
  case I8085::JP:
  case I8085::JM:
  case I8085::JNZ:
  case I8085::JC:
  case I8085::JNC:
    return MI.getOperand(0).getMBB(); 
  }
}


bool I8085InstrInfo::isBranchOffsetInRange(unsigned BranchOp,
                                         int64_t BrOffset) const {

  switch (BranchOp) {
  default:
    llvm_unreachable("unexpected opcode!");
  case I8085::JMP:
  case I8085::JZ:
  case I8085::JP:
  case I8085::JM:
  case I8085::JNZ:
  case I8085::JC:
  case I8085::JNC:
    return true;
  }
}

} // end of namespace llvm
