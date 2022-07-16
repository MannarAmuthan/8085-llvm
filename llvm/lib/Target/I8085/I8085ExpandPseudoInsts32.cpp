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


#define DEBUG_TYPE "i8085-expand-psuedo-32"


using namespace llvm;

#define I8085_EXPAND_PSEUDO_32_NAME "I8085 pseudo instruction expansion pass for instructions with 32 bit imaginary registers"

namespace {

/// Expands "placeholder" instructions which uses 32 bit imaginary registers marked as pseudo into
/// actual I8085 instructions.
class I8085ExpandPseudo32 : public MachineFunctionPass {
public:
  static char ID;

  I8085ExpandPseudo32() : MachineFunctionPass(ID) {
    initializeI8085ExpandPseudo32Pass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return I8085_EXPAND_PSEUDO_32_NAME; }

private:
  typedef MachineBasicBlock Block;
  typedef Block::iterator BlockIt;

  const I8085RegisterInfo *TRI;
  const TargetInstrInfo *TII;


  bool expandMBB(Block &MBB);
  bool expandMI(Block &MBB, BlockIt MBBI);
  template <unsigned OP> bool expand(Block &MBB, BlockIt MBBI);
  bool binOperationWithImmediateOperand(unsigned opCode, Block &MBB, BlockIt MBBI);
  bool binOperation(unsigned opCode, Block &MBB, BlockIt MBBI);

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

char I8085ExpandPseudo32::ID = 0;

bool I8085ExpandPseudo32::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  BlockIt MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    BlockIt NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool I8085ExpandPseudo32::runOnMachineFunction(MachineFunction &MF) {

  LLVM_DEBUG({
    dbgs() << "********** Expand 32 bit register pseudo instructions **********\n"
           << "********** Function: " << MF.getName() << '\n';
  });

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

uint8_t high(uint64_t input){return (input >> 8) & 0xFF;}

uint8_t low(uint64_t input){return input & 0xFF;}

bool I8085ExpandPseudo32::binOperationWithImmediateOperand(unsigned opCode, Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne; 
  uint64_t immToAdd = MI.getOperand(2).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  uint8_t nibbleOne = immToAdd & 0x000000FF  ;
  uint8_t nibbleTwo = (immToAdd >> 8) & 0x000000FF  ;
  uint8_t nibbleThree = (immToAdd >> 16) & 0x000000FF  ;
  uint8_t nibbleFour = (immToAdd >> 24) & 0x000000FF  ;

  uint8_t values[]={nibbleOne,nibbleTwo,nibbleThree,nibbleFour};

  if(destReg==I8085::IBX){  
    index=4; 
  }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      buildMI(MBB, MBBI, opCode).addImm(values[i]);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

bool I8085ExpandPseudo32::binOperation(unsigned opCode, Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne; 
  unsigned operandTwo = MI.getOperand(2).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  
      index=4; 
  }

  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+4]);

      buildMI(MBB, MBBI, opCode);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::XOR_32>(Block &MBB, BlockIt MBBI) {
  return binOperation(I8085::XRA_M,MBB,MBBI);
}
template <> bool I8085ExpandPseudo32::expand<I8085::OR_32>(Block &MBB, BlockIt MBBI) {
  return binOperation(I8085::ORA_M,MBB,MBBI);
}
template <> bool I8085ExpandPseudo32::expand<I8085::AND_32>(Block &MBB, BlockIt MBBI) {
  return binOperation(I8085::ANA_M,MBB,MBBI);
}

template <> bool I8085ExpandPseudo32::expand<I8085::XORI_32>(Block &MBB, BlockIt MBBI) {
  return binOperationWithImmediateOperand(I8085::XRI,MBB,MBBI);
}
template <> bool I8085ExpandPseudo32::expand<I8085::ORI_32>(Block &MBB, BlockIt MBBI) {
  return binOperationWithImmediateOperand(I8085::ORI,MBB,MBBI);
}
template <> bool I8085ExpandPseudo32::expand<I8085::ANDI_32>(Block &MBB, BlockIt MBBI) {
  return binOperationWithImmediateOperand(I8085::ANI,MBB,MBBI);
}


template <> bool I8085ExpandPseudo32::expand<I8085::RR_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){
      index=4;
  }

  for(int i=3;i>-1;i--){
    buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+i]);
    buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
    buildMI(MBB, MBBI, I8085::RAR);
    buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
  buildMI(MBB, MBBI, I8085::ANI).addImm(127);  
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  
  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::RL_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){
      index=4;
  }

  for(int i=0;i<4;i++){
    buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+i]);
    buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
    buildMI(MBB, MBBI, I8085::RAL);
    buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
  buildMI(MBB, MBBI, I8085::ANI).addImm(254);  
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::SEXT32_INREG_8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);

  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

  buildMI(MBB, MBBI, I8085::ADI)
    .addImm(128); // 80h in hex .. [Adding 80h will set carry flag if HSB is high]

  buildMI(MBB, MBBI, I8085::SBB)
    .addReg(I8085::A);  // will result in FFh if CF set, 0 else

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+2]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::SEXT32_INREG_16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);

  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

  buildMI(MBB, MBBI, I8085::ADI)
    .addImm(128); // 80h in hex .. [Adding 80h will set carry flag if HSB is high]

  buildMI(MBB, MBBI, I8085::SBB)
    .addReg(I8085::A);  // will result in FFh if CF set, 0 else

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+2]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A); 

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::TRUNC32TO16>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();

  unsigned destLow,destHigh;

  if(destReg==I8085::BC){  destLow=I8085::C;  destHigh=I8085::B; }
  if(destReg==I8085::DE){  destLow=I8085::E;  destHigh=I8085::D; }

  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(destLow,RegState::Define);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(destHigh,RegState::Define);

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::SEXT16TO32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();

  unsigned opOneLow,opOneHigh;

  if(srcReg==I8085::BC){  opOneLow=I8085::C;  opOneHigh=I8085::B; }
  if(srcReg==I8085::DE){  opOneLow=I8085::E;  opOneHigh=I8085::D; }

  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(opOneLow);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(opOneHigh);

  buildMI(MBB, MBBI, I8085::MOV).addReg(I8085::A,RegState::Define).addReg(opOneHigh);

  buildMI(MBB, MBBI, I8085::ADI)
    .addImm(128); // 80h in hex .. [Adding 80h will set carry flag if HSB is high]

  buildMI(MBB, MBBI, I8085::SBB)
    .addReg(I8085::A);  // will result in FFh if CF set, 0 else


  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+2]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A); 

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::ZEXT16TO32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();

  unsigned opOneLow,opOneHigh;

  if(srcReg==I8085::BC){  opOneLow=I8085::C;  opOneHigh=I8085::B; }
  if(srcReg==I8085::DE){  opOneLow=I8085::E;  opOneHigh=I8085::D; }

  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MVI_M).addImm(0);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+2]);
  buildMI(MBB, MBBI, I8085::MVI_M).addImm(0);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(opOneHigh);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(opOneLow);       

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::AEXT16TO32>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  return expand<I8085::ZEXT16TO32>(MBB, MI);
}


template <> bool I8085ExpandPseudo32::expand<I8085::TRUNC32TO8>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(destReg,RegState::Define);

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::SEXT8TO32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(srcReg);

  buildMI(MBB, MBBI, I8085::MOV).addReg(I8085::A,RegState::Define).addReg(srcReg);

  buildMI(MBB, MBBI, I8085::ADI)
    .addImm(128); // 80h in hex .. [Adding 80h will set carry flag if HSB is high]

  buildMI(MBB, MBBI, I8085::SBB)
    .addReg(I8085::A);  // will result in FFh if CF set, 0 else


  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+2]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::ZEXT8TO32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();

  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  
      index=4; 
  }

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+3]);
  buildMI(MBB, MBBI, I8085::MVI_M).addImm(0);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+2]);
  buildMI(MBB, MBBI, I8085::MVI_M).addImm(0);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index+1]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(0);
  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[index]);
  buildMI(MBB, MBBI, I8085::MOV_M).addReg(srcReg);       

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::AEXT8TO32>(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  return expand<I8085::ZEXT8TO32>(MBB, MI);
}

template <> bool I8085ExpandPseudo32::expand<I8085::MOV_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned srcReg = MI.getOperand(1).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};

  if(srcReg==I8085::IBX){ 
          for(int i=0;i<4;i++){
              buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+4]);
              buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

              buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
              buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
          }  
  }
  else{
          for(int i=0;i<4;i++){
              buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
              buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

              buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+4]);
              buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
          }   
  }         

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::STORE_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned baseReg = MI.getOperand(0).getReg();
  unsigned offsetToStore = MI.getOperand(1).getImm();

  unsigned srcReg = MI.getOperand(2).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){  
    index=4; 
  }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(offsetToStore+i);
      buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::ADD_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne; 
  unsigned operandTwo = MI.getOperand(2).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  
    index=4; 
  }
  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+4]);

      if(i>0) buildMI(MBB, MBBI, I8085::ADC_M);
      else buildMI(MBB, MBBI, I8085::ADD_M);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::SUBI_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne; 
  uint64_t immToAdd = MI.getOperand(2).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  uint8_t nibbleOne = immToAdd & 0x000000FF  ;
  uint8_t nibbleTwo = (immToAdd >> 8) & 0x000000FF  ;
  uint8_t nibbleThree = (immToAdd >> 16) & 0x000000FF  ;
  uint8_t nibbleFour = (immToAdd >> 24) & 0x000000FF  ;

  uint8_t values[]={nibbleOne,nibbleTwo,nibbleThree,nibbleFour};

  if(destReg==I8085::IBX){  
    index=4; 
  }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      if(i>0){ 
        buildMI(MBB, MBBI, I8085::SBI).addImm(values[i]);
      }
      else {
        buildMI(MBB, MBBI, I8085::SUI).addImm(values[i]);
      }

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::SUB_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne; 
  unsigned operandTwo = MI.getOperand(2).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int indexOne = 0,indexTwo=4;

  if(destReg==I8085::IBX){  
    indexOne=4;
    indexTwo=0;
  }
  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+indexOne]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+indexTwo]);

      if(i>0) buildMI(MBB, MBBI, I8085::SBB_M);
      else buildMI(MBB, MBBI, I8085::SUB_M);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+indexOne]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::ADDI_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned destReg = operandOne; 
  uint64_t immToAdd = MI.getOperand(2).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  uint8_t nibbleOne = immToAdd & 0x000000FF  ;
  uint8_t nibbleTwo = (immToAdd >> 8) & 0x000000FF  ;
  uint8_t nibbleThree = (immToAdd >> 16) & 0x000000FF  ;
  uint8_t nibbleFour = (immToAdd >> 24) & 0x000000FF  ;

  uint8_t values[]={nibbleOne,nibbleTwo,nibbleThree,nibbleFour};

  if(destReg==I8085::IBX){  
    index=4; 
  }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      if(i>0){ 
        buildMI(MBB, MBBI, I8085::ACI).addImm(values[i]);
      }
      else {
        buildMI(MBB, MBBI, I8085::ADI).addImm(values[i]);
      }

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::LOAD_32_WITH_ADDR>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned baseReg = MI.getOperand(1).getReg();
  uint16_t offsetToLoad = MI.getOperand(2).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  index=4; }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(offsetToLoad+i);
      buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::LOAD_32>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  uint64_t immToLoad = MI.getOperand(1).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  uint8_t nibbleOne = immToLoad & 0x000000FF  ;
  uint8_t nibbleTwo = (immToLoad >> 8) & 0x000000FF  ;
  uint8_t nibbleThree = (immToLoad >> 16) & 0x000000FF  ;
  uint8_t nibbleFour = (immToLoad >> 24) & 0x000000FF  ;

  uint8_t values[]={nibbleOne,nibbleTwo,nibbleThree,nibbleFour};

  if(destReg==I8085::IBX){  index=4; }
  
  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MVI_M).addImm(values[i]);
  }            

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::JMP_32_IF_NOT_EQUAL>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(0).getReg();
  unsigned operandTwo = MI.getOperand(1).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};

  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+4]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A, RegState::Define);
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i]);
      buildMI(MBB, MBBI, I8085::CMP_M);
      buildMI(MBB, MBBI, I8085::JNZ).addMBB(MI.getOperand(2).getMBB());
  }            

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::JMP_32_IF_SAME_SIGN>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(0).getReg();
  unsigned operandTwo = MI.getOperand(1).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;


  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[4]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A, RegState::Define);

  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[0]);

  buildMI(MBB, MBBI, I8085::XRA_M);
  buildMI(MBB, MBBI, I8085::ANI).addImm(128);  
  buildMI(MBB, MBBI, I8085::JZ).addMBB(MI.getOperand(2).getMBB());
        
  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::JMP_32_IF_POSITIVE>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned operandOne = MI.getOperand(0).getReg();
  
  int address[]={11,12,13,14,15,16,17,18};
  int higherByteIndex = 3;

  if(operandOne==I8085::IBX){  higherByteIndex=7; }


  buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[higherByteIndex]);
  buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A, RegState::Define);
  buildMI(MBB, MBBI, I8085::ANI).addImm(128);  
  buildMI(MBB, MBBI, I8085::JZ).addMBB(MI.getOperand(1).getMBB());
        
  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::STORE_32_AT_OFFSET_WITH_SP>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned srcReg = MI.getOperand(0).getReg();
  unsigned offsetToStore = MI.getOperand(1).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(srcReg==I8085::IBX){  
    index=4; 
  }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);

      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(offsetToStore+i);
      buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}


template <> bool I8085ExpandPseudo32::expand<I8085::LOAD_32_OFFSET_WITH_SP>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();
  uint16_t offsetToLoad = MI.getOperand(1).getImm();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  index=4; }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(offsetToLoad+i);
      buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

template <> bool I8085ExpandPseudo32::expand<I8085::LOAD_32_WITH_IMM_ADDR>(Block &MBB, BlockIt MBBI) {
  const I8085Subtarget &STI = MBB.getParent()->getSubtarget<I8085Subtarget>();
  MachineInstr &MI = *MBBI;

  unsigned destReg = MI.getOperand(0).getReg();

  const GlobalValue* amount = MI.getOperand(1).getGlobal();
  
  int address[]={11,12,13,14,15,16,17,18};
  int index = 0;

  if(destReg==I8085::IBX){  index=4; }

  
  for(int i=0;i<4;i++){
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addGlobalAddress(amount,i);
      buildMI(MBB, MBBI, I8085::DAD).addReg(I8085::SP);
      buildMI(MBB, MBBI, I8085::MOV_FROM_M).addReg(I8085::A,RegState::Define);
      buildMI(MBB, MBBI, I8085::LXI).addReg(I8085::H,RegState::Define).addImm(address[i+index]);
      buildMI(MBB, MBBI, I8085::MOV_M).addReg(I8085::A);
  }            

  MI.eraseFromParent();
  return true;
}

bool I8085ExpandPseudo32::expandMI(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  int Opcode = MBBI->getOpcode();

#define EXPAND(Op)                                                             \
  case Op:                                                                     \
    return expand<Op>(MBB, MI)

  switch (Opcode) {
    EXPAND(I8085::XORI_32);
    EXPAND(I8085::ORI_32);
    EXPAND(I8085::ANDI_32);
    EXPAND(I8085::XOR_32);
    EXPAND(I8085::OR_32);
    EXPAND(I8085::AND_32);
    EXPAND(I8085::RL_32);
    EXPAND(I8085::RR_32);
    EXPAND(I8085::SEXT32_INREG_16);
    EXPAND(I8085::SEXT32_INREG_8);
    EXPAND(I8085::TRUNC32TO16);
    EXPAND(I8085::SEXT16TO32);
    EXPAND(I8085::AEXT16TO32);
    EXPAND(I8085::ZEXT16TO32);
    EXPAND(I8085::TRUNC32TO8);
    EXPAND(I8085::SEXT8TO32);
    EXPAND(I8085::AEXT8TO32);
    EXPAND(I8085::ZEXT8TO32);
    EXPAND(I8085::LOAD_32_OFFSET_WITH_SP);
    EXPAND(I8085::STORE_32_AT_OFFSET_WITH_SP);
    EXPAND(I8085::JMP_32_IF_POSITIVE);
    EXPAND(I8085::JMP_32_IF_SAME_SIGN);
    EXPAND(I8085::JMP_32_IF_NOT_EQUAL);
    EXPAND(I8085::MOV_32);
    EXPAND(I8085::SUBI_32);
    EXPAND(I8085::SUB_32);
    EXPAND(I8085::ADD_32);
    EXPAND(I8085::ADDI_32);
    EXPAND(I8085::LOAD_32);
    EXPAND(I8085::STORE_32);
    EXPAND(I8085::LOAD_32_WITH_ADDR);
    EXPAND(I8085::LOAD_32_WITH_IMM_ADDR);
  }
#undef EXPAND
  return false;
}

} // end of anonymous namespace

INITIALIZE_PASS(I8085ExpandPseudo32, "i8085-expand-pseudo32", I8085_EXPAND_PSEUDO_32_NAME,
                false, false)
namespace llvm { 

FunctionPass *createI8085ExpandPseudo32Pass() {  return new I8085ExpandPseudo32();  }

} // end of namespace llvm