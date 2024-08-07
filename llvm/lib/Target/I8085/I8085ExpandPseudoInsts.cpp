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
#include <stdint.h>

#include <iostream>

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

  MachineRegisterInfo &getRegInfo(Block &MBB) {
    return MBB.getParent()->getRegInfo();
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
      assert(ExpandCount < 10 && "pseudo expand limit reached");

      bool BlockModified = expandMBB(MBB);
      Modified |= BlockModified;
      ExpandCount++;

      ContinueExpanding = BlockModified;
    } while (ContinueExpanding);
  }

  return Modified;
}

template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_16_ADDR_CONTENT>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcLowReg,srcHighReg;
  unsigned destLowReg,destHighReg;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();

  if(srcReg==I8085::BC){srcLowReg=I8085::C; srcHighReg=I8085::B;}
  if(srcReg==I8085::DE){srcLowReg=I8085::E; srcHighReg=I8085::D;}

  if(destReg==I8085::BC){destLowReg=I8085::C; destHighReg=I8085::B;}
  if(destReg==I8085::DE){destLowReg=I8085::E; destHighReg=I8085::D;}

  buildMI(MBB, MBBI,  I8085::MOV).addReg(I8085::H, RegState::Define).addReg(srcHighReg);
  buildMI(MBB, MBBI,  I8085::MOV).addReg(I8085::L, RegState::Define).addReg(srcLowReg);
  buildMI(MBB, MBBI,  I8085::MOV_FROM_M).addReg(destLowReg ,RegState::Define);
  buildMI(MBB, MBBI,  I8085::INX).addReg(I8085::H, RegState::Define);
  buildMI(MBB, MBBI,  I8085::MOV_FROM_M).addReg(destHighReg ,RegState::Define);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_8_ADDR_CONTENT>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned lowReg,highReg;
  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();

  if(srcReg==I8085::BC){lowReg=I8085::C; highReg=I8085::B;}
  if(srcReg==I8085::DE){lowReg=I8085::E; highReg=I8085::D;}

  buildMI(MBB, MBBI,  I8085::MOV).addReg(I8085::H, RegState::Define).addReg(highReg);
  buildMI(MBB, MBBI,  I8085::MOV).addReg(I8085::L, RegState::Define).addReg(lowReg);
  buildMI(MBB, MBBI,  I8085::MOV_FROM_M).addReg(destReg ,RegState::Define);
  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::STORE_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;


  unsigned srcReg = MI.getOperand(2).getReg();
  int64_t offsetToStore = MI.getOperand(1).getImm();

  /*  Getting address to store the register */

  buildMI(MBB, MBBI, I8085::LXI)
      .addReg(I8085::H,RegState::Define)
      .addImm(offsetToStore);

  buildMI(MBB, MBBI, I8085::DAD)
      .addReg(I8085::SP);
  
  /* Store the register value pointed by HL reg */
  
  buildMI(MBB, MBBI, I8085::MOV_M)
      .addReg(srcReg);
  
  MI.eraseFromParent();
  return true;
}


uint8_t high(uint64_t input){return (input >> 8) & 0xFF;}

uint8_t low(uint64_t input){return input & 0xFF;}

template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned lowReg,highReg;
  unsigned destReg = MI.getOperand(0).getReg();

  if(destReg==I8085::BC){
    lowReg=I8085::C;
    highReg=I8085::B;
  }

  if(destReg==I8085::DE){
    lowReg=I8085::E;
    highReg=I8085::D;
  }

  uint64_t amount = MI.getOperand(1).getImm();


  buildMI(MBB, MBBI,  I8085::MVI)
        .addReg(highReg, RegState::Define)
        .addImm(high(amount));

  buildMI(MBB, MBBI,  I8085::MVI)
        .addReg(lowReg, RegState::Define)
        .addImm(low(amount));

  MI.eraseFromParent();

  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_8_WITH_IMM_ADDR>(Block &MBB, BlockIt MBBI) {
const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned lowReg,highReg;
  unsigned destReg = MI.getOperand(0).getReg();

  const GlobalValue* amount = MI.getOperand(1).getGlobal();

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addGlobalAddress(amount);
  buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(destReg,RegState::Define);

  MI.eraseFromParent();

  return true;
}


template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_16_WITH_IMM_ADDR>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned lowReg,highReg;
  unsigned destReg = MI.getOperand(0).getReg();

  if(destReg==I8085::BC){
    lowReg=I8085::C;
    highReg=I8085::B;
  }

  if(destReg==I8085::DE){
    lowReg=I8085::E;
    highReg=I8085::D;
  }

  const GlobalValue* amount = MI.getOperand(1).getGlobal();

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addGlobalAddress(amount,1);
  buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(highReg,RegState::Define);

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addGlobalAddress(amount);
  buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(lowReg,RegState::Define);

  MI.eraseFromParent();

  return true;
}


template <>
bool I8085ExpandPseudo::expand<I8085::STORE_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned lowReg,highReg;
  unsigned baseReg = MI.getOperand(0).getReg();
  int64_t offsetToStore = MI.getOperand(1).getImm();
  unsigned destReg = MI.getOperand(2).getReg();
  
  if(destReg==I8085::BC){  lowReg=I8085::C;  highReg=I8085::B; }
  if(destReg==I8085::DE){  lowReg=I8085::E;  highReg=I8085::D; }

  buildMI(MBB, MBBI,  I8085::STORE_8)
        .addReg(baseReg)
        .addImm(offsetToStore+1)
        .addReg(highReg);

  buildMI(MBB, MBBI,  I8085::STORE_8)
        .addReg(baseReg)
        .addImm(offsetToStore)
        .addReg(lowReg);    

  MI.eraseFromParent();

  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::SHRINK_STACK_BY>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  int64_t Amount = MI.getOperand(0).getImm();

  buildMI(MBB, MBBI,  I8085::LXI)
        .addReg(I8085::H,RegState::Define)
        .addImm(Amount);

  buildMI(MBB, MBBI,  I8085::DAD)
        .addReg(I8085::SP);

  buildMI(MBB, MBBI,  I8085::SPHL);
  
  MI.eraseFromParent();
  return true;
}

uint16_t twos_complement(uint16_t val) { return -(unsigned int)val;}

template <>
bool I8085ExpandPseudo::expand<I8085::GROW_STACK_BY>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  uint16_t Amount = twos_complement((uint8_t) MI.getOperand(0).getImm());
  
  buildMI(MBB, MBBI,  I8085::LXI)
        .addReg(I8085::H,RegState::Define)
        .addImm(Amount);

  buildMI(MBB, MBBI,  I8085::DAD)
        .addReg(I8085::SP);

  buildMI(MBB, MBBI,  I8085::SPHL);
  
  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_8_WITH_ADDR>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
 

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned baseReg = MI.getOperand(1).getReg();
  uint16_t offsetToLoad = MI.getOperand(2).getImm();
  
    /*  Getting address to store the register */

  buildMI(MBB, MBBI, I8085::LXI)
      .addReg(I8085::H,RegState::Define)
      .addImm(offsetToLoad);

  buildMI(MBB, MBBI, I8085::DAD)
      .addReg(I8085::SP);
  
  /* Store the register value pointed by HL reg */
  
  buildMI(MBB, MBBI, I8085::MOV_FROM_M)
      .addReg(destReg,RegState::Define);
  
  MI.eraseFromParent();
  return true;
}


template <>
bool I8085ExpandPseudo::expand<I8085::LOAD_16_WITH_ADDR>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned lowReg,highReg;
  unsigned destReg = MI.getOperand(0).getReg();
  unsigned baseReg = MI.getOperand(1).getReg();
  uint16_t offsetToLoad = MI.getOperand(2).getImm();
  
  if(destReg==I8085::BC){  lowReg=I8085::C;  highReg=I8085::B; }
  if(destReg==I8085::DE){  lowReg=I8085::E;  highReg=I8085::D; }

  buildMI(MBB, MBBI, I8085::LOAD_8_WITH_ADDR)
      .addReg(highReg)
      .addReg(baseReg)
      .addImm(offsetToLoad+1);

  buildMI(MBB, MBBI, I8085::LOAD_8_WITH_ADDR)
      .addReg(lowReg)
      .addReg(baseReg)
      .addImm(offsetToLoad);    
  
  MI.eraseFromParent();
  return true;
}


template <>
bool I8085ExpandPseudo::expand<I8085::ANDI_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  int64_t immValue = MI.getOperand(2).getImm();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A,RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::ANI)
      .addImm(immValue);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg,RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::ORI_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  int64_t immValue = MI.getOperand(2).getImm();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A,RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::ORI)
      .addImm(immValue);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg,RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::XORI_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  int64_t immValue = MI.getOperand(2).getImm();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A,RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::XRI)
      .addImm(immValue);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg,RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}



template <>
bool I8085ExpandPseudo::expand<I8085::AND_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A,RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::ANA)
      .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg,RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::OR_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A,RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::ORA)
      .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg,RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::XOR_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A,RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::XRA)
      .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg,RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::AND_16>(Block &MBB, BlockIt MBBI) {
const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead = MI.getOperand(0).isDead();
  
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::AND_8)
      .addReg(destLow, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destLow)
      .addReg(opLow);

  buildMI(MBB, MBBI, I8085::AND_8)
      .addReg(destHigh, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destHigh)
      .addReg(opHigh);    

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::OR_16>(Block &MBB, BlockIt MBBI) {
const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead = MI.getOperand(0).isDead();
  
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::OR_8)
      .addReg(destLow, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destLow)
      .addReg(opLow);

  buildMI(MBB, MBBI, I8085::OR_8)
      .addReg(destHigh, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destHigh)
      .addReg(opHigh);    
      
  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::XOR_16>(Block &MBB, BlockIt MBBI) {
const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead = MI.getOperand(0).isDead();
  
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::XOR_8)
      .addReg(destLow, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destLow)
      .addReg(opLow);

  buildMI(MBB, MBBI, I8085::XOR_8)
      .addReg(destHigh, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destHigh)
      .addReg(opHigh);    
      
  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::ADD_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();    
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A, RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::ADD)
      .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg, RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}


template <>
bool I8085ExpandPseudo::expand<I8085::SUB_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();   
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A, RegState::Define)
      .addReg(operandOne);

  buildMI(MBB, MBBI, I8085::SUB)
      .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destReg, RegState::Define)
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <>
bool I8085ExpandPseudo::expand<I8085::ADD_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead = MI.getOperand(0).isDead();
  
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::ADD_8)
      .addReg(destLow, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destLow)
      .addReg(opLow);
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A, RegState::Define)
      .addReg(destHigh);

  buildMI(MBB, MBBI, I8085::ADC)
      .addReg(opHigh);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destHigh, RegState::Define)
      .addReg(I8085::A);


  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::SUB_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  unsigned operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead=MI.getOperand(0).isDead();
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::SUB_8)
      .addReg(destLow, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(destLow)
      .addReg(opLow);
  
  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(I8085::A, RegState::Define)
      .addReg(destHigh);

  buildMI(MBB, MBBI, I8085::SBB)
      .addReg(opHigh);

  buildMI(MBB, MBBI, I8085::MOV)
      .addReg(destHigh, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::SET_EQ_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(0).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead=MI.getOperand(0).isDead();

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(destReg);

  buildMI(MBB, MBBI, I8085::SUB)
    .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::CMA);  

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destReg,RegState::Define)
    .addReg(I8085::A);
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::SET_NE_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(0).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead=MI.getOperand(0).isDead();

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(destReg);

  buildMI(MBB, MBBI, I8085::SUB)
    .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destReg,RegState::Define)
    .addReg(I8085::A);
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::SET_EQ_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead=MI.getOperand(0).isDead();
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }

  buildMI(MBB, MBBI, I8085::SUB_16)
    .addReg(destReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(operandOne)
    .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::CMA);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destReg,RegState::Define)
    .addReg(I8085::A);
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::SET_NE_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne;
  uint16_t operandTwo = MI.getOperand(2).getReg();  
  bool DstIsDead=MI.getOperand(0).isDead();
  unsigned opLow,opHigh;
  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opLow=I8085::C;  opHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opLow=I8085::E;  opHigh=I8085::D; }

  buildMI(MBB, MBBI, I8085::SUB_16)
    .addReg(destReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(operandOne)
    .addReg(operandTwo);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destReg,RegState::Define)
    .addReg(I8085::A);
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::JMP_8_IF>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operand = MI.getOperand(0).getReg();


  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(operand);

  buildMI(MBB, MBBI, I8085::ORI)
    .addImm(0);

  buildMI(MBB, MBBI, I8085::JNZ).add(MI.getOperand(1));
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::TRUNC16TO8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned destReg = MI.getOperand(0).getReg();
  unsigned operand = MI.getOperand(1).getReg();

  unsigned operandLow,operandHigh;
  if(operand==I8085::BC){  operandLow=I8085::C;  operandHigh=I8085::B; }
  if(operand==I8085::DE){  operandLow=I8085::E;  operandHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destReg,RegState::Define)
    .addReg(operandLow);
     
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::SEXT8TO16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned destReg = MI.getOperand(0).getReg();
  unsigned operand = MI.getOperand(1).getReg();

  unsigned destLow,destHigh;
  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(operand);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destLow,RegState::Define)
    .addReg(I8085::A);

  buildMI(MBB, MBBI, I8085::ADI)
    .addImm(128); // 80h in hex .. [Adding 80h will set carry flag if HSB is high]


  buildMI(MBB, MBBI, I8085::SBB)
    .addReg(I8085::A);  // will result in FFh if CF set, 0 else

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destHigh,RegState::Define)
    .addReg(I8085::A);  
     
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::ZEXT8TO16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned destReg = MI.getOperand(0).getReg();
  unsigned operand = MI.getOperand(1).getReg();

  unsigned destLow,destHigh;
  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(operand);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(destLow,RegState::Define)
    .addReg(I8085::A);
  
  buildMI(MBB, MBBI, I8085::MVI)
    .addReg(destHigh,RegState::Define)
    .addImm(0);
     
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::AEXT8TO16>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  return expand<I8085::ZEXT8TO16>(MBB, MI);
}

template <> bool I8085ExpandPseudo::expand<I8085::RL_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned reg = MI.getOperand(0).getReg();

  unsigned regLow,regHigh;
  if(reg==I8085::BC){  regLow=I8085::C;  regHigh=I8085::B; }
  if(reg==I8085::DE){  regLow=I8085::E;  regHigh=I8085::D; }
  
  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(regLow);

  buildMI(MBB, MBBI, I8085::RAL);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(regLow,RegState::Define)
    .addReg(I8085::A);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(regHigh);

  buildMI(MBB, MBBI, I8085::RAL);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(regHigh,RegState::Define)
    .addReg(I8085::A);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(regLow);

  buildMI(MBB, MBBI, I8085::ANI)
    .addImm(254);  

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(regLow,RegState::Define)
    .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo::expand<I8085::RR_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned reg = MI.getOperand(0).getReg();

  unsigned regLow,regHigh;
  if(reg==I8085::BC){  regLow=I8085::C;  regHigh=I8085::B; }
  if(reg==I8085::DE){  regLow=I8085::E;  regHigh=I8085::D; }

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(regHigh);

  buildMI(MBB, MBBI, I8085::RAR);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(regHigh,RegState::Define)
    .addReg(I8085::A);


  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(regLow);

  buildMI(MBB, MBBI, I8085::RAR);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(regLow,RegState::Define)
    .addReg(I8085::A);


  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(regHigh);

  buildMI(MBB, MBBI, I8085::ANI)
    .addImm(127);  

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(regHigh,RegState::Define)
    .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::RL_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned reg = MI.getOperand(0).getReg();

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(reg);

  buildMI(MBB, MBBI, I8085::ADD)
    .addReg(I8085::A);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(reg,RegState::Define)
    .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo::expand<I8085::RR_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned reg = MI.getOperand(0).getReg();

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(reg);

  buildMI(MBB, MBBI, I8085::RRC);

  buildMI(MBB, MBBI, I8085::ANI)
    .addImm(127);

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(reg,RegState::Define)
    .addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::JMP_16_IF>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(0).getReg();

  unsigned opOneLow,opOneHigh;

  if(operandOne==I8085::BC){  opOneLow=I8085::C;  opOneHigh=I8085::B; }
  if(operandOne==I8085::DE){  opOneLow=I8085::E;  opOneHigh=I8085::D; }

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(opOneHigh);

  buildMI(MBB, MBBI, I8085::ANI)
    .addImm(255);

  buildMI(MBB, MBBI, I8085::JNZ)
    .addMBB(MI.getOperand(1).getMBB());

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(opOneLow);

  buildMI(MBB, MBBI, I8085::ANI)
    .addImm(255);

  buildMI(MBB, MBBI, I8085::JNZ)
    .addMBB(MI.getOperand(1).getMBB());
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::JMP_16_IF_NOT_EQUAL>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(0).getReg();
  unsigned operandTwo = MI.getOperand(1).getReg();

  unsigned opOneLow,opOneHigh;
  unsigned opTwoLow,opTwoHigh;

  if(operandOne==I8085::BC){  opOneLow=I8085::C;  opOneHigh=I8085::B; }
  if(operandOne==I8085::DE){  opOneLow=I8085::E;  opOneHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opTwoLow=I8085::C;  opTwoHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opTwoLow=I8085::E;  opTwoHigh=I8085::D; }

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(opOneHigh);

  buildMI(MBB, MBBI, I8085::CMP)
    .addReg(opTwoHigh);

  buildMI(MBB, MBBI, I8085::JNZ)
    .addMBB(MI.getOperand(2).getMBB());

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(opOneLow);

  buildMI(MBB, MBBI, I8085::CMP)
    .addReg(opTwoLow);

  buildMI(MBB, MBBI, I8085::JNZ)
    .addMBB(MI.getOperand(2).getMBB());
  
  MI.eraseFromParent();
  return true;
}



template <> bool I8085ExpandPseudo::expand<I8085::JMP_16_IF_SAME_SIGN>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(0).getReg();
  unsigned operandTwo = MI.getOperand(1).getReg();

  unsigned opOneLow,opOneHigh;
  unsigned opTwoLow,opTwoHigh;

  if(operandOne==I8085::BC){  opOneLow=I8085::C;  opOneHigh=I8085::B; }
  if(operandOne==I8085::DE){  opOneLow=I8085::E;  opOneHigh=I8085::D; }

  if(operandTwo==I8085::BC){  opTwoLow=I8085::C;  opTwoHigh=I8085::B; }
  if(operandTwo==I8085::DE){  opTwoLow=I8085::E;  opTwoHigh=I8085::D; }

  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(opOneHigh);

  buildMI(MBB, MBBI, I8085::XRA)
    .addReg(opTwoHigh);

  buildMI(MBB, MBBI, I8085::ANI)
        .addImm(128);  

  buildMI(MBB, MBBI, I8085::JZ)
    .addMBB(MI.getOperand(2).getMBB());
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::JMP_16_IF_POSITIVE>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;
  
  unsigned operandOne = MI.getOperand(0).getReg();

  unsigned opOneLow,opOneHigh;

  if(operandOne==I8085::BC){  opOneLow=I8085::C;  opOneHigh=I8085::B; }
  if(operandOne==I8085::DE){  opOneLow=I8085::E;  opOneHigh=I8085::D; }


  buildMI(MBB, MBBI, I8085::MOV)
    .addReg(I8085::A,RegState::Define)
    .addReg(opOneHigh);

  buildMI(MBB, MBBI, I8085::ANI)
        .addImm(128);  

  buildMI(MBB, MBBI, I8085::JZ)
    .addMBB(MI.getOperand(1).getMBB());
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::STORE_8_AT_OFFSET_WITH_SP>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  int64_t offsetToStore = MI.getOperand(1).getImm();
  
  buildMI(MBB, MBBI, I8085::STORE_8)
    .addReg(I8085::SP)
    .addImm(offsetToStore)
    .addReg(srcReg);
  
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo::expand<I8085::STORE_16_AT_OFFSET_WITH_SP>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  int64_t offsetToStore = MI.getOperand(1).getImm();
  
  buildMI(MBB, MBBI, I8085::STORE_16)
    .addReg(I8085::SP)
    .addImm(offsetToStore)
    .addReg(srcReg);
  
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
    EXPAND(I8085::LOAD_16_ADDR_CONTENT);
    EXPAND(I8085::LOAD_8_ADDR_CONTENT);
    EXPAND(I8085::STORE_16_AT_OFFSET_WITH_SP);
    EXPAND(I8085::STORE_8_AT_OFFSET_WITH_SP);
    EXPAND(I8085::JMP_16_IF_POSITIVE);
    EXPAND(I8085::JMP_16_IF_SAME_SIGN);
    EXPAND(I8085::JMP_16_IF_NOT_EQUAL);
    EXPAND(I8085::JMP_8_IF);
    EXPAND(I8085::TRUNC16TO8);
    EXPAND(I8085::AEXT8TO16);
    EXPAND(I8085::SEXT8TO16);
    EXPAND(I8085::ZEXT8TO16);
    EXPAND(I8085::RL_16);
    EXPAND(I8085::RR_16);
    EXPAND(I8085::RL_8);
    EXPAND(I8085::RR_8);
    // EXPAND(I8085::SET_NE_16);
    // EXPAND(I8085::SET_EQ_16);
    // EXPAND(I8085::SET_NE_8);
    // EXPAND(I8085::SET_EQ_8);
    EXPAND(I8085::XOR_16);
    EXPAND(I8085::OR_16);
    EXPAND(I8085::AND_16);
    EXPAND(I8085::XOR_8);
    EXPAND(I8085::OR_8);
    EXPAND(I8085::AND_8);
    EXPAND(I8085::XORI_8);
    EXPAND(I8085::ORI_8);
    EXPAND(I8085::ANDI_8);
    EXPAND(I8085::SUB_16);
    EXPAND(I8085::ADD_16);
    EXPAND(I8085::SUB_8);
    EXPAND(I8085::ADD_8);
    EXPAND(I8085::LOAD_16_WITH_ADDR);
    EXPAND(I8085::LOAD_8_WITH_ADDR);
    EXPAND(I8085::LOAD_8_WITH_IMM_ADDR);
    EXPAND(I8085::LOAD_16_WITH_IMM_ADDR);
    EXPAND(I8085::LOAD_16);
    EXPAND(I8085::STORE_16);
    EXPAND(I8085::SHRINK_STACK_BY);
    EXPAND(I8085::GROW_STACK_BY);
    EXPAND(I8085::STORE_8);
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