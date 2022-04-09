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

  // Reserve the intermediate result registers r1 and r2
  // The result of instructions like 'mul' is always stored here.
  // R0/R1/R1R0 are always reserved on both avr and avrtiny.
  Reserved.set(I8085::R0);
  Reserved.set(I8085::R1);
  Reserved.set(I8085::R1R0);

  // Reserve the stack pointer.
  Reserved.set(I8085::SPL);
  Reserved.set(I8085::SPH);
  Reserved.set(I8085::SP);

  // Reserve R2~R17 only on avrtiny.
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
    return &I8085::DREGSRegClass;
  }

  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    return &I8085::GPR8RegClass;
  }

  if (TRI->isTypeLegalForClass(*RC, MVT::i8)) {
    return &I8085::GR8RegClass;
  }

  llvm_unreachable("Invalid register size");
}

/// Fold a frame offset shared between two add instructions into a single one.
static void foldFrameOffset(MachineBasicBlock::iterator &II, int &Offset,
                            Register DstReg) {
  MachineInstr &MI = *II;
  int Opcode = MI.getOpcode();

  // Don't bother trying if the next instruction is not an add or a sub.
  if ((Opcode != I8085::SUBIWRdK) && (Opcode != I8085::ADIWRdK)) {
    return;
  }

  // Check that DstReg matches with next instruction, otherwise the instruction
  // is not related to stack address manipulation.
  if (DstReg != MI.getOperand(0).getReg()) {
    return;
  }

  // Add the offset in the next instruction to our offset.
  switch (Opcode) {
  case I8085::SUBIWRdK:
    Offset += -MI.getOperand(2).getImm();
    break;
  case I8085::ADIWRdK:
    Offset += MI.getOperand(2).getImm();
    break;
  }

  // Finally remove the instruction.
  II++;
  MI.eraseFromParent();
}

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
  Offset += MFI.getStackSize() - TFI->getOffsetOfLocalArea() + 1;
  // Fold incoming offset.
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  // std::cout << Offset << "\n";
  // std::cout << "############" << "\n";

  // This is actually "load effective address" of the stack slot
  // instruction. We have only two-address instructions, thus we need to
  // expand it into move + add.
  if (MI.getOpcode() == I8085::FRMIDX) {
    MI.setDesc(TII.get(I8085::MOVWRdRr));
    MI.getOperand(FIOperandNum).ChangeToRegister(I8085::D, false);
    MI.removeOperand(2);

    assert(Offset > 0 && "Invalid offset");

    // We need to materialize the offset via an add instruction.
    unsigned Opcode;
    Register DstReg = MI.getOperand(0).getReg();
    assert(DstReg != I8085::L && "Dest reg cannot be the frame pointer");

    II++; // Skip over the FRMIDX (and now MOVW) instruction.

    // Generally, to load a frame address two add instructions are emitted that
    // could get folded into a single one:
    //  movw    r31:r30, r29:r28
    //  adiw    r31:r30, 29
    //  adiw    r31:r30, 16
    // to:
    //  movw    r31:r30, r29:r28
    //  adiw    r31:r30, 45
    if (II != MBB.end())
      foldFrameOffset(II, Offset, DstReg);

    // Select the best opcode based on DstReg and the offset size.
    switch (DstReg) {
    case I8085::R25R24:
    case I8085::R27R26:
    case I8085::R31R30: {
      if (isUInt<6>(Offset)) {
        Opcode = I8085::ADIWRdK;
        break;
      }
      LLVM_FALLTHROUGH;
    }
    default: {
      // This opcode will get expanded into a pair of subi/sbci.
      Opcode = I8085::SUBIWRdK;
      Offset = -Offset;
      break;
    }
    }

    MachineInstr *New = BuildMI(MBB, II, dl, TII.get(Opcode), DstReg)
                            .addReg(DstReg, RegState::Kill)
                            .addImm(Offset);
    New->getOperand(3).setIsDead();

    return;
  }

  // If the offset is too big we have to adjust and restore the frame pointer
  // to materialize a valid load/store with displacement.
  //: TODO: consider using only one adiw/sbiw chain for more than one frame
  //: index
  if (Offset > 62) {
    unsigned AddOpc = I8085::ADIWRdK, SubOpc = I8085::SBIWRdK;
    int AddOffset = Offset - 63 + 1;

    // For huge offsets where adiw/sbiw cannot be used use a pair of subi/sbci.
    if ((Offset - 63 + 1) > 63) {
      AddOpc = I8085::SUBIWRdK;
      SubOpc = I8085::SUBIWRdK;
      AddOffset = -AddOffset;
    }

    // It is possible that the spiller places this frame instruction in between
    // a compare and branch, invalidating the contents of SREG set by the
    // compare instruction because of the add/sub pairs. Conservatively save and
    // restore SREG before and after each add/sub pair.
    BuildMI(MBB, II, dl, TII.get(I8085::INRdA), I8085::R0)
        .addImm(STI.getIORegSREG());

    MachineInstr *New = BuildMI(MBB, II, dl, TII.get(AddOpc), I8085::L)
                            .addReg(I8085::L, RegState::Kill)
                            .addImm(AddOffset);
    New->getOperand(3).setIsDead();

    // Restore SREG.
    BuildMI(MBB, std::next(II), dl, TII.get(I8085::OUTARr))
        .addImm(STI.getIORegSREG())
        .addReg(I8085::R0, RegState::Kill);

    // No need to set SREG as dead here otherwise if the next instruction is a
    // cond branch it will be using a dead register.
    BuildMI(MBB, std::next(II), dl, TII.get(SubOpc), I8085::L)
        .addReg(I8085::L, RegState::Kill)
        .addImm(Offset - 63 + 1);

    Offset = 62;
  }

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
  // FIXME: Currently we're using avr-gcc as reference, so we restrict
  // ptrs to Y and Z regs. Though avr-gcc has buggy implementation
  // of memory constraint, so we can fix it and bit avr-gcc here ;-)
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
