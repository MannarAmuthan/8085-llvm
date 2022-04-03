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
  const I8085RegisterInfo &TRI = *STI.getRegisterInfo();
  unsigned Opc;

  // Not all I8085 devices support the 16-bit `MOVW` instruction.
  if (I8085::GR8RegClass.contains(DestReg, SrcReg)) {
    if (STI.hasMOVW() && I8085::DREGSMOVWRegClass.contains(DestReg, SrcReg)) {
      BuildMI(MBB, MI, DL, get(I8085::MOVWRdRr), DestReg)
          .addReg(SrcReg, getKillRegState(KillSrc));
    } else {
      Register DestLo, DestHi, SrcLo, SrcHi;

      TRI.splitReg(DestReg, DestLo, DestHi);
      TRI.splitReg(SrcReg, SrcLo, SrcHi);

      // Copy each individual register with the `MOV` instruction.
      BuildMI(MBB, MI, DL, get(I8085::MOV), DestLo)
          .addReg(SrcLo, getKillRegState(KillSrc));
      BuildMI(MBB, MI, DL, get(I8085::MOV), DestHi)
          .addReg(SrcHi, getKillRegState(KillSrc));
    }
  } else {
    if (I8085::GR8RegClass.contains(DestReg, SrcReg)) {
      Opc = I8085::MOV;
    } else if (SrcReg == I8085::SP && I8085::GR8RegClass.contains(DestReg)) {
      Opc = I8085::SPREAD;
    } else if (DestReg == I8085::SP && I8085::GR8RegClass.contains(SrcReg)) {
      Opc = I8085::SPWRITE;
    } else {
      // std::cout << SrcReg << "\n";
      // std::cout <<  DestReg << "\n";
      // llvm_unreachable("Impossible reg-to-reg copy");
      Opc = I8085::MOV;
    }

     

    BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
}

unsigned I8085InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                           int &FrameIndex) const {
  switch (MI.getOpcode()) {
  case I8085::LDDRdPtrQ:
  case I8085::LDDWRdYQ: { //: FIXME: remove this once PR13375 gets fixed
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
  case I8085::STDPtrQRr:
  case I8085::STDWPtrQRr: {
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
                                       const TargetRegisterInfo *TRI) const {
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
    Opcode = I8085::STDPtrQRr;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    Opcode = I8085::STDWPtrQRr;
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
                                        const TargetRegisterInfo *TRI) const {
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
    Opcode = I8085::LDDRdPtrQ;
  } else if (TRI->isTypeLegalForClass(*RC, MVT::i16)) {
    // Opcode = I8085::LDDWRdPtrQ;
    //: FIXME: remove this once PR13375 gets fixed
    Opcode = I8085::LDDWRdYQ;
  } else {
    llvm_unreachable("Cannot load this register from a stack slot!");
  }

  BuildMI(MBB, MI, DL, get(Opcode), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(MMO);
}

const MCInstrDesc &I8085InstrInfo::getBrCond(I8085CC::CondCodes CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Unknown condition code!");
  case I8085CC::COND_EQ:
    return get(I8085::BREQk);
  case I8085CC::COND_NE:
    return get(I8085::BRNEk);
  case I8085CC::COND_GE:
    return get(I8085::BRGEk);
  case I8085CC::COND_LT:
    return get(I8085::BRLTk);
  case I8085CC::COND_SH:
    return get(I8085::BRSHk);
  case I8085CC::COND_LO:
    return get(I8085::BRLOk);
  case I8085CC::COND_MI:
    return get(I8085::BRMIk);
  case I8085CC::COND_PL:
    return get(I8085::BRPLk);
  }
}

I8085CC::CondCodes I8085InstrInfo::getCondFromBranchOpc(unsigned Opc) const {
  switch (Opc) {
  default:
    return I8085CC::COND_INVALID;
  case I8085::BREQk:
    return I8085CC::COND_EQ;
  case I8085::BRNEk:
    return I8085CC::COND_NE;
  case I8085::BRSHk:
    return I8085CC::COND_SH;
  case I8085::BRLOk:
    return I8085CC::COND_LO;
  case I8085::BRMIk:
    return I8085CC::COND_MI;
  case I8085::BRPLk:
    return I8085CC::COND_PL;
  case I8085::BRGEk:
    return I8085CC::COND_GE;
  case I8085::BRLTk:
    return I8085CC::COND_LT;
  }
}

I8085CC::CondCodes I8085InstrInfo::getOppositeCondition(I8085CC::CondCodes CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Invalid condition!");
  case I8085CC::COND_EQ:
    return I8085CC::COND_NE;
  case I8085CC::COND_NE:
    return I8085CC::COND_EQ;
  case I8085CC::COND_SH:
    return I8085CC::COND_LO;
  case I8085CC::COND_LO:
    return I8085CC::COND_SH;
  case I8085CC::COND_GE:
    return I8085CC::COND_LT;
  case I8085CC::COND_LT:
    return I8085CC::COND_GE;
  case I8085CC::COND_MI:
    return I8085CC::COND_PL;
  case I8085CC::COND_PL:
    return I8085CC::COND_MI;
  }
}

bool I8085InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                 MachineBasicBlock *&TBB,
                                 MachineBasicBlock *&FBB,
                                 SmallVectorImpl<MachineOperand> &Cond,
                                 bool AllowModify) const {
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  MachineBasicBlock::iterator UnCondBrIter = MBB.end();

  while (I != MBB.begin()) {
    --I;
    if (I->isDebugInstr()) {
      continue;
    }

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(*I)) {
      break;
    }

    // A terminator that isn't a branch can't easily be handled
    // by this analysis.
    if (!I->getDesc().isBranch()) {
      return true;
    }

    // Handle unconditional branches.
    //: TODO: add here jmp
    if (I->getOpcode() == I8085::RJMPk) {
      UnCondBrIter = I;

      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a JMP, delete them.
      while (std::next(I) != MBB.end()) {
        std::next(I)->eraseFromParent();
      }

      Cond.clear();
      FBB = nullptr;

      // Delete the JMP if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
        TBB = nullptr;
        I->eraseFromParent();
        I = MBB.end();
        UnCondBrIter = MBB.end();
        continue;
      }

      // TBB is used to indicate the unconditinal destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }

    // Handle conditional branches.
    I8085CC::CondCodes BranchCode = getCondFromBranchOpc(I->getOpcode());
    if (BranchCode == I8085CC::COND_INVALID) {
      return true; // Can't handle indirect branch.
    }

    // Working from the bottom, handle the first conditional branch.
    if (Cond.empty()) {
      MachineBasicBlock *TargetBB = I->getOperand(0).getMBB();
      if (AllowModify && UnCondBrIter != MBB.end() &&
          MBB.isLayoutSuccessor(TargetBB)) {
        // If we can modify the code and it ends in something like:
        //
        //     jCC L1
        //     jmp L2
        //   L1:
        //     ...
        //   L2:
        //
        // Then we can change this to:
        //
        //     jnCC L2
        //   L1:
        //     ...
        //   L2:
        //
        // Which is a bit more efficient.
        // We conditionally jump to the fall-through block.
        BranchCode = getOppositeCondition(BranchCode);
        unsigned JNCC = getBrCond(BranchCode).getOpcode();
        MachineBasicBlock::iterator OldInst = I;

        BuildMI(MBB, UnCondBrIter, MBB.findDebugLoc(I), get(JNCC))
            .addMBB(UnCondBrIter->getOperand(0).getMBB());
        BuildMI(MBB, UnCondBrIter, MBB.findDebugLoc(I), get(I8085::RJMPk))
            .addMBB(TargetBB);

        OldInst->eraseFromParent();
        UnCondBrIter->eraseFromParent();

        // Restart the analysis.
        UnCondBrIter = MBB.end();
        I = MBB.end();
        continue;
      }

      FBB = TBB;
      TBB = I->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(BranchCode));
      continue;
    }

    // Handle subsequent conditional branches. Only handle the case where all
    // conditional branches branch to the same destination.
    assert(Cond.size() == 1);
    assert(TBB);

    // Only handle the case where all conditional branches branch to
    // the same destination.
    if (TBB != I->getOperand(0).getMBB()) {
      return true;
    }

    I8085CC::CondCodes OldBranchCode = (I8085CC::CondCodes)Cond[0].getImm();
    // If the conditions are the same, we can leave them alone.
    if (OldBranchCode == BranchCode) {
      continue;
    }

    return true;
  }

  return false;
}

unsigned I8085InstrInfo::insertBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *TBB,
                                    MachineBasicBlock *FBB,
                                    ArrayRef<MachineOperand> Cond,
                                    const DebugLoc &DL, int *BytesAdded) const {
  if (BytesAdded)
    *BytesAdded = 0;

  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 1 || Cond.size() == 0) &&
         "I8085 branch conditions have one component!");

  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch with multiple successors!");
    auto &MI = *BuildMI(&MBB, DL, get(I8085::RJMPk)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    return 1;
  }

  // Conditional branch.
  unsigned Count = 0;
  I8085CC::CondCodes CC = (I8085CC::CondCodes)Cond[0].getImm();
  auto &CondMI = *BuildMI(&MBB, DL, getBrCond(CC)).addMBB(TBB);

  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(CondMI);
  ++Count;

  if (FBB) {
    // Two-way Conditional branch. Insert the second branch.
    auto &MI = *BuildMI(&MBB, DL, get(I8085::RJMPk)).addMBB(FBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    ++Count;
  }

  return Count;
}

unsigned I8085InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                    int *BytesRemoved) const {
  if (BytesRemoved)
    *BytesRemoved = 0;

  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) {
    --I;
    if (I->isDebugInstr()) {
      continue;
    }
    //: TODO: add here the missing jmp instructions once they are implemented
    // like jmp, {e}ijmp, and other cond branches, ...
    if (I->getOpcode() != I8085::RJMPk &&
        getCondFromBranchOpc(I->getOpcode()) == I8085CC::COND_INVALID) {
      break;
    }

    // Remove the branch.
    if (BytesRemoved)
      *BytesRemoved += getInstSizeInBytes(*I);
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

bool I8085InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 1 && "Invalid I8085 branch condition!");

  I8085CC::CondCodes CC = static_cast<I8085CC::CondCodes>(Cond[0].getImm());
  Cond[0].setImm(getOppositeCondition(CC));

  return false;
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
  case TargetOpcode::INLINEASM:
  case TargetOpcode::INLINEASM_BR: {
    const MachineFunction &MF = *MI.getParent()->getParent();
    const I8085TargetMachine &TM =
        static_cast<const I8085TargetMachine &>(MF.getTarget());
    const I8085Subtarget &STI = MF.getSubtarget<I8085Subtarget>();
    const TargetInstrInfo &TII = *STI.getInstrInfo();

    return TII.getInlineAsmLength(MI.getOperand(0).getSymbolName(),
                                  *TM.getMCAsmInfo());
  }
  }
}

MachineBasicBlock *
I8085InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("unexpected opcode!");
  case I8085::JMPk:
  case I8085::CALLk:
  case I8085::RCALLk:
  case I8085::RJMPk:
  case I8085::BREQk:
  case I8085::BRNEk:
  case I8085::BRSHk:
  case I8085::BRLOk:
  case I8085::BRMIk:
  case I8085::BRPLk:
  case I8085::BRGEk:
  case I8085::BRLTk:
    return MI.getOperand(0).getMBB();
  case I8085::BRBSsk:
  case I8085::BRBCsk:
    return MI.getOperand(1).getMBB();
  case I8085::SBRCRrB:
  case I8085::SBRSRrB:
  case I8085::SBICAb:
  case I8085::SBISAb:
    llvm_unreachable("unimplemented branch instructions");
  }
}

bool I8085InstrInfo::isBranchOffsetInRange(unsigned BranchOp,
                                         int64_t BrOffset) const {

  switch (BranchOp) {
  default:
    llvm_unreachable("unexpected opcode!");
  case I8085::JMPk:
  case I8085::CALLk:
    return true;
  case I8085::RCALLk:
  case I8085::RJMPk:
    return isIntN(13, BrOffset);
  case I8085::BRBSsk:
  case I8085::BRBCsk:
  case I8085::BREQk:
  case I8085::BRNEk:
  case I8085::BRSHk:
  case I8085::BRLOk:
  case I8085::BRMIk:
  case I8085::BRPLk:
  case I8085::BRGEk:
  case I8085::BRLTk:
    return isIntN(7, BrOffset);
  }
}

void I8085InstrInfo::insertIndirectBranch(MachineBasicBlock &MBB,
                                        MachineBasicBlock &NewDestBB,
                                        MachineBasicBlock &RestoreBB,
                                        const DebugLoc &DL, int64_t BrOffset,
                                        RegScavenger *RS) const {
  // This method inserts a *direct* branch (JMP), despite its name.
  // LLVM calls this method to fixup unconditional branches; it never calls
  // insertBranch or some hypothetical "insertDirectBranch".
  // See lib/CodeGen/RegisterRelaxation.cpp for details.
  // We end up here when a jump is too long for a RJMP instruction.
  BuildMI(&MBB, DL, get(I8085::JMPk)).addMBB(&NewDestBB);
}

} // end of namespace llvm
