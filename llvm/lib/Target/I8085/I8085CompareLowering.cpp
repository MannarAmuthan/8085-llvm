//===-- I8085ISelLowering.cpp - I8085 DAG Lowering Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that I8085 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "I8085ISelLowering.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"

#include <iostream>

#include "I8085.h"
#include "I8085MachineFunctionInfo.h"
#include "I8085Subtarget.h"
#include "I8085TargetMachine.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

namespace llvm {


MachineBasicBlock *I8085TargetLowering::insertCond8Set(MachineInstr &MI,
                                                  MachineBasicBlock *MBB) const {

  int Opc = MI.getOpcode();
  const I8085InstrInfo &TII = (const I8085InstrInfo &)*MI.getParent()
                                ->getParent()
                                ->getSubtarget()
                                .getInstrInfo();

  DebugLoc dl = MI.getDebugLoc();

  // To "insert" a SELECT instruction, we insert the diamond
  // control-flow pattern. The incoming instruction knows the
  // destination vreg to set, the condition code register to branch
  // on, the true/false values to select between, and a branch opcode
  // to use.

  MachineFunction *MF = MBB->getParent();
  
  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineBasicBlock *FallThrough = MBB->getFallThrough();

  // If the current basic block falls through to another basic block,
  // we must insert an unconditional branch to the fallthrough destination
  // if we are to insert basic blocks at the prior fallthrough point.
  if (FallThrough != nullptr) {
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(FallThrough);
  }

  MachineBasicBlock *trueMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *falseMBB = MF->CreateMachineBasicBlock(LLVM_BB);

  MachineFunction::iterator I;
  for (I = MF->begin(); I != MF->end() && &(*I) != MBB; ++I)
    ;
  if (I != MF->end())
    ++I;
  MF->insert(I, trueMBB);
  MF->insert(I, falseMBB);

  // Transfer remaining instructions and all successors of the current
  // block to the block which will contain the Phi node for the
  // select.
  trueMBB->splice(trueMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());

  trueMBB->transferSuccessorsAndUpdatePHIs(MBB);

  unsigned operandOne = MI.getOperand(1).getReg(); 
  unsigned operandTwo = MI.getOperand(2).getReg();
  
  
  unsigned tempRegOne = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));
  unsigned tempRegTwo = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));



  unsigned destReg = MI.getOperand(0).getReg();

  BuildMI(MBB, dl, TII.get(I8085::MOV))
        .addReg(I8085::A, RegState::Define)
        .addReg(operandOne);

  BuildMI(MBB, dl, TII.get(I8085::SUB))
        .addReg(operandTwo);

  BuildMI(MBB, dl, TII.get(I8085::MVI))
        .addReg(tempRegOne, RegState::Define)
        .addImm(1);      

  if(Opc == I8085::SET_EQ_8){
    BuildMI(MBB, dl, TII.get(I8085::JZ)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_NE_8){
    BuildMI(MBB, dl, TII.get(I8085::JNZ)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_UGT_8 ){
    BuildMI(MBB, dl, TII.get(I8085::JZ)).addMBB(falseMBB);
    BuildMI(MBB, dl, TII.get(I8085::JNC)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_ULT_8){
    BuildMI(MBB, dl, TII.get(I8085::JC)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_UGE_8 ){
    BuildMI(MBB, dl, TII.get(I8085::JC)).addMBB(falseMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(trueMBB);  
  }

  else if(Opc == I8085::SET_ULE_8 ){
    BuildMI(MBB, dl, TII.get(I8085::JZ)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JP)).addMBB(falseMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(trueMBB); 
  }

  
  MBB->addSuccessor(falseMBB);
  MBB->addSuccessor(trueMBB);

  // Unconditionally flow back to the true block
  BuildMI(falseMBB, dl, TII.get(I8085::MVI))
        .addReg(tempRegTwo, RegState::Define)
        .addImm(0);

  BuildMI(falseMBB, dl, TII.get(I8085::JMP))
            .addMBB(trueMBB);

  falseMBB->addSuccessor(trueMBB);
  
  BuildMI(*trueMBB, trueMBB->begin(), dl, TII.get(I8085::PHI),destReg)
      .addReg(tempRegOne)
      .addMBB(MBB)
      .addReg(tempRegTwo)
      .addMBB(falseMBB);

  MI.eraseFromParent();
  return trueMBB;
}


MachineBasicBlock *I8085TargetLowering::insertSigned8Cond(MachineInstr &MI,
                                                  MachineBasicBlock *MBB) const {

  int Opc = MI.getOpcode();
  const I8085InstrInfo &TII = (const I8085InstrInfo &)*MI.getParent()
                                ->getParent()
                                ->getSubtarget()
                                .getInstrInfo();

  DebugLoc dl = MI.getDebugLoc();

  // To "insert" a SELECT instruction, we insert the diamond
  // control-flow pattern. The incoming instruction knows the
  // destination vreg to set, the condition code register to branch
  // on, the true/false values to select between, and a branch opcode
  // to use.

  MachineFunction *MF = MBB->getParent();
  
  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineBasicBlock *FallThrough = MBB->getFallThrough();

  // If the current basic block falls through to another basic block,
  // we must insert an unconditional branch to the fallthrough destination
  // if we are to insert basic blocks at the prior fallthrough point.
  if (FallThrough != nullptr) {
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(FallThrough);
  }

  MachineBasicBlock *continMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *samesignMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *diffsignMBB = MF->CreateMachineBasicBlock(LLVM_BB);

  MachineFunction::iterator I;
  for (I = MF->begin(); I != MF->end() && &(*I) != MBB; ++I)
    ;
  if (I != MF->end())
    ++I;
  MF->insert(I, continMBB);
  MF->insert(I, samesignMBB);
  MF->insert(I, diffsignMBB);

  // Transfer remaining instructions and all successors of the current
  // block to the block which will contain the Phi node for the
  // select.
  continMBB->splice(continMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());

  continMBB->transferSuccessorsAndUpdatePHIs(MBB);

  unsigned operandOne = MI.getOperand(1).getReg(); 
  unsigned operandTwo = MI.getOperand(2).getReg();
  
  
  unsigned tempRegOne = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));
  unsigned tempRegTwo = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));



  unsigned destReg = MI.getOperand(0).getReg();

  BuildMI(MBB, dl, TII.get(I8085::MOV))
        .addReg(I8085::A, RegState::Define)
        .addReg(operandOne);

  BuildMI(MBB, dl, TII.get(I8085::XRA))
        .addReg(operandTwo);
  
  BuildMI(MBB, dl, TII.get(I8085::JZ))
        .addMBB(samesignMBB);

  BuildMI(MBB, dl, TII.get(I8085::JNZ))
        .addMBB(diffsignMBB);
    
  
  if(Opc == I8085::SET_GT_8 ){
    BuildMI(samesignMBB, dl, TII.get(I8085::SET_SAME_SIGN_GT_8)) .addReg(tempRegOne, RegState::Define).addReg(operandOne).addReg(operandTwo);
    BuildMI(diffsignMBB, dl, TII.get(I8085::SET_DIFF_SIGN_GT_8)).addReg(tempRegTwo, RegState::Define).addReg(operandOne).addReg(operandTwo); 
  }

  else if(Opc == I8085::SET_LT_8){
    BuildMI(samesignMBB, dl, TII.get(I8085::SET_SAME_SIGN_LT_8)) .addReg(tempRegOne, RegState::Define).addReg(operandOne).addReg(operandTwo);
    BuildMI(diffsignMBB, dl, TII.get(I8085::SET_DIFF_SIGN_LT_8)).addReg(tempRegTwo, RegState::Define).addReg(operandOne).addReg(operandTwo);   
  }

  else if(Opc == I8085::SET_GE_8 ){
    BuildMI(samesignMBB, dl, TII.get(I8085::SET_SAME_SIGN_GE_8)) .addReg(tempRegOne, RegState::Define).addReg(operandOne).addReg(operandTwo);
    BuildMI(diffsignMBB, dl, TII.get(I8085::SET_DIFF_SIGN_GE_8)).addReg(tempRegTwo, RegState::Define).addReg(operandOne).addReg(operandTwo);  
  }

  else if(Opc == I8085::SET_LE_8 ){
    BuildMI(samesignMBB, dl, TII.get(I8085::SET_SAME_SIGN_LE_8)) .addReg(tempRegOne, RegState::Define).addReg(operandOne).addReg(operandTwo);
    BuildMI(diffsignMBB, dl, TII.get(I8085::SET_DIFF_SIGN_LE_8)).addReg(tempRegTwo, RegState::Define).addReg(operandOne).addReg(operandTwo); 
  }

  BuildMI(samesignMBB, dl, TII.get(I8085::JMP)) .addMBB(continMBB);
  BuildMI(diffsignMBB, dl, TII.get(I8085::JMP)) .addMBB(continMBB); 

  
  MBB->addSuccessor(samesignMBB);
  MBB->addSuccessor(diffsignMBB);

  samesignMBB->addSuccessor(continMBB);
  diffsignMBB->addSuccessor(continMBB);
  
  BuildMI(*continMBB, continMBB->begin(), dl, TII.get(I8085::PHI),destReg)
      .addReg(tempRegOne)
      .addMBB(samesignMBB)
      .addReg(tempRegTwo)
      .addMBB(diffsignMBB);

  MI.eraseFromParent();
  return continMBB;
}

MachineBasicBlock *I8085TargetLowering::insertCond16Set(MachineInstr &MI,
                                                  MachineBasicBlock *MBB) const {

  int Opc = MI.getOpcode();
  const I8085InstrInfo &TII = (const I8085InstrInfo &)*MI.getParent()
                                ->getParent()
                                ->getSubtarget()
                                .getInstrInfo();

  DebugLoc dl = MI.getDebugLoc();

  // To "insert" a SELECT instruction, we insert the diamond
  // control-flow pattern. The incoming instruction knows the
  // destination vreg to set, the condition code register to branch
  // on, the true/false values to select between, and a branch opcode
  // to use.

  MachineFunction *MF = MBB->getParent();
  
  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineBasicBlock *FallThrough = MBB->getFallThrough();

  // If the current basic block falls through to another basic block,
  // we must insert an unconditional branch to the fallthrough destination
  // if we are to insert basic blocks at the prior fallthrough point.
  if (FallThrough != nullptr) {
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(FallThrough);
  }

  MachineBasicBlock *trueMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *falseMBB = MF->CreateMachineBasicBlock(LLVM_BB);

  MachineFunction::iterator I;
  for (I = MF->begin(); I != MF->end() && &(*I) != MBB; ++I)
    ;
  if (I != MF->end())
    ++I;
  MF->insert(I, trueMBB);
  MF->insert(I, falseMBB);

  // Transfer remaining instructions and all successors of the current
  // block to the block which will contain the Phi node for the
  // select.
  trueMBB->splice(trueMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());

  trueMBB->transferSuccessorsAndUpdatePHIs(MBB);

  unsigned operandOne = MI.getOperand(1).getReg(); 
  unsigned operandTwo = MI.getOperand(2).getReg();
  
  
  unsigned tempRegOne = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));
  unsigned tempRegTwo = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));

  unsigned tempRegThree = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i16));



  unsigned destReg = MI.getOperand(0).getReg();

  BuildMI(MBB, dl, TII.get(I8085::SUB_16))
        .addReg(tempRegThree, RegState::Define)
        .addReg(operandOne)
        .addReg(operandTwo);

  BuildMI(MBB, dl, TII.get(I8085::MVI))
        .addReg(tempRegOne, RegState::Define)
        .addImm(1);      

  if(Opc == I8085::SET_EQ_16){
    BuildMI(MBB, dl, TII.get(I8085::JZ)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_NE_16){
    BuildMI(MBB, dl, TII.get(I8085::JNZ)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_GT_16){
    BuildMI(MBB, dl, TII.get(I8085::JNC)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_LT_16){
    BuildMI(MBB, dl, TII.get(I8085::JC)).addMBB(trueMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(falseMBB);  
  }

  else if(Opc == I8085::SET_GE_16){
    BuildMI(MBB, dl, TII.get(I8085::JC)).addMBB(falseMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(trueMBB);  
  }

  else if(Opc == I8085::SET_LE_16){
    BuildMI(MBB, dl, TII.get(I8085::JNC)).addMBB(falseMBB);
    BuildMI(MBB, dl, TII.get(I8085::JMP)).addMBB(trueMBB); 
  }

  
  MBB->addSuccessor(falseMBB);
  MBB->addSuccessor(trueMBB);

  // Unconditionally flow back to the true block
  BuildMI(falseMBB, dl, TII.get(I8085::MVI))
        .addReg(tempRegTwo, RegState::Define)
        .addImm(0);

  BuildMI(falseMBB, dl, TII.get(I8085::JMP))
            .addMBB(trueMBB);

  falseMBB->addSuccessor(trueMBB);
  
  BuildMI(*trueMBB, trueMBB->begin(), dl, TII.get(I8085::PHI),destReg)
      .addReg(tempRegOne)
      .addMBB(MBB)
      .addReg(tempRegTwo)
      .addMBB(falseMBB);

  MI.eraseFromParent();
  return trueMBB;
}


} 