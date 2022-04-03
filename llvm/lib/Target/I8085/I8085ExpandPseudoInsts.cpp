//===-- I8085ExpandPseudoInsts.cpp - Expand pseudo instructions -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass that expands pseudo instructions into target
// instructions. This pass should be run after register allocation but before
// the post-regalloc scheduling pass.
//
//===----------------------------------------------------------------------===//

#include "I8085.h"
#include "I8085InstrInfo.h"
#include "I8085TargetMachine.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

using namespace llvm;

#define I8085_EXPAND_PSEUDO_NAME "I8085 pseudo instruction expansion pass"

namespace {

/// Expands "placeholder" instructions marked as pseudo into
/// actual I8085 instructions.
class I8085ExpandPseudo : public MachineFunctionPass {
public:
  static char ID;

  I8085ExpandPseudo() : MachineFunctionPass(ID) {
    initializeI8085ExpandPseudoPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;


  StringRef getPassName() const override { return I8085_EXPAND_PSEUDO_NAME; }

private:
  typedef MachineBasicBlock Block;
  typedef Block::iterator BlockIt;

  const I8085RegisterInfo *TRI;
  const TargetInstrInfo *TII;

  bool expandMBB(Block &MBB);
  bool expandMI(Block &MBB, BlockIt MBBI);

  template <unsigned OP> bool expand(Block &MBB, BlockIt MBBI);

  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode));
  }

  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode,
                              Register DstReg) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode), DstReg);
  }
  
};

char I8085ExpandPseudo::ID = 0;

bool I8085ExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  BlockIt MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    BlockIt NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool I8085ExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;

  const I8085Subtarget &STI = MF.getSubtarget<I8085Subtarget>();
  TRI = STI.getRegisterInfo();
  TII = STI.getInstrInfo();

  // We need to track liveness in order to use register scavenging.
  MF.getProperties().set(MachineFunctionProperties::Property::TracksLiveness);

  for (Block &MBB : MF) {
    bool ContinueExpanding = true;
    unsigned ExpandCount = 0;

    // Continue expanding the block until all pseudos are expanded.
    do {
      assert(ExpandCount < 100 && "pseudo expand limit reached");

      bool BlockModified = expandMBB(MBB);
      Modified |= BlockModified;
      ExpandCount++;

      ContinueExpanding = BlockModified;
    } while (ContinueExpanding);
  }

  return Modified;
}


template <>
bool I8085ExpandPseudo::expand<I8085::SPREAD>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  Register DstReg = MI.getOperand(0).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  unsigned Flags = MI.getFlags();
  unsigned OpLo = I8085::INRdA;
  unsigned OpHi = I8085::INRdA;

  // Low part
  // buildMI(MBB, MBBI, OpLo)
  //     .addReg(DstReg, RegState::Define | getDeadRegState(DstIsDead))
  //     .addImm(0x3d)
  //     .setMIFlags(Flags);

  MI.eraseFromParent();
  return true;
}


bool I8085ExpandPseudo::expandMI(Block &MBB, BlockIt MBBI) {
MachineInstr &MI = *MBBI;
int Opcode = MBBI->getOpcode();

#define EXPAND(Op)                                                             \
  case Op:                                                                     \
    return expand<Op>(MBB, MI)

  switch (Opcode) {
    EXPAND(I8085::SPREAD);
  }
#undef EXPAND
  return false;
}


} // end of anonymous namespace

INITIALIZE_PASS(I8085ExpandPseudo, "i8085-expand-pseudo", I8085_EXPAND_PSEUDO_NAME,
                false, false)
namespace llvm {

FunctionPass *createI8085ExpandPseudoPass() { return new I8085ExpandPseudo(); }

} // end of namespace llvm
