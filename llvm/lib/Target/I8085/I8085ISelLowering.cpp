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

#include "I8085.h"
#include "I8085MachineFunctionInfo.h"
#include "I8085Subtarget.h"
#include "I8085TargetMachine.h"
#include "MCTargetDesc/I8085MCTargetDesc.h"

#include <iostream>

namespace llvm {

I8085TargetLowering::I8085TargetLowering(const I8085TargetMachine &TM,
                                     const I8085Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  // Set up the register classes.
  addRegisterClass(MVT::i8, &I8085::GPR8RegClass);

  addRegisterClass(MVT::i8, &I8085::GR8RegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(Subtarget.getRegisterInfo());

  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);
  setSchedulingPreference(Sched::RegPressure);
  setStackPointerRegisterToSaveRestore(I8085::SP);
  setSupportsUnalignedAtomics(true);

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i16, Custom);

  setMinFunctionAlignment(Align(2));
  setMinimumJumpTableEntries(UINT_MAX);
}

const char *I8085TargetLowering::getTargetNodeName(unsigned Opcode) const {
#define NODE(name)                                                             \
  case I8085ISD::name:                                                           \
    return #name

  switch (Opcode) {
  default:
    return nullptr;
    NODE(RET_FLAG);
    NODE(RETI_FLAG);
    NODE(CALL);
    NODE(WRAPPER);
    NODE(LSL);
    NODE(LSR);
    NODE(ROL);
    NODE(ROR);
    NODE(ASR);
    NODE(LSLLOOP);
    NODE(LSRLOOP);
    NODE(ROLLOOP);
    NODE(RORLOOP);
    NODE(ASRLOOP);
    NODE(BRCOND);
    NODE(CMP);
    NODE(CMPC);
    NODE(TST);
    NODE(SELECT_CC);
#undef NODE
  }
}

EVT I8085TargetLowering::getSetCCResultType(const DataLayout &DL, LLVMContext &,
                                          EVT VT) const {
  assert(!VT.isVector() && "No I8085 SetCC type for vectors!");
  return MVT::i8;
}

SDValue I8085TargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  auto DL = DAG.getDataLayout();

  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result =
      DAG.getTargetGlobalAddress(GV, SDLoc(Op), getPointerTy(DL), Offset);
  return DAG.getNode(I8085ISD::WRAPPER, SDLoc(Op), getPointerTy(DL), Result);
}

SDValue I8085TargetLowering::LowerBlockAddress(SDValue Op,
                                             SelectionDAG &DAG) const {
  auto DL = DAG.getDataLayout();
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();

  SDValue Result = DAG.getTargetBlockAddress(BA, getPointerTy(DL));

  return DAG.getNode(I8085ISD::WRAPPER, SDLoc(Op), getPointerTy(DL), Result);
}

/// IntCCToI8085CC - Convert a DAG integer condition code to an I8085 CC.
static I8085CC::CondCodes intCCToI8085CC(ISD::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("Unknown condition code!");
  case ISD::SETEQ:
    return I8085CC::COND_EQ;
  case ISD::SETNE:
    return I8085CC::COND_NE;
  case ISD::SETGE:
    return I8085CC::COND_GE;
  case ISD::SETLT:
    return I8085CC::COND_LT;
  case ISD::SETUGE:
    return I8085CC::COND_SH;
  case ISD::SETULT:
    return I8085CC::COND_LO;
  }
}

/// Returns appropriate CP/CPI/CPC nodes code for the given 8/16-bit operands.
SDValue I8085TargetLowering::getI8085Cmp(SDValue LHS, SDValue RHS,
                                     SelectionDAG &DAG, SDLoc DL) const {
  assert((LHS.getSimpleValueType() == RHS.getSimpleValueType()) &&
         "LHS and RHS have different types");
  assert(((LHS.getSimpleValueType() == MVT::i16) ||
          (LHS.getSimpleValueType() == MVT::i8)) &&
         "invalid comparison type");

  SDValue Cmp;

  if (LHS.getSimpleValueType() == MVT::i16 && isa<ConstantSDNode>(RHS)) {
    // Generate a CPI/CPC pair if RHS is a 16-bit constant.
    SDValue LHSlo = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, LHS,
                                DAG.getIntPtrConstant(0, DL));
    SDValue LHShi = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, LHS,
                                DAG.getIntPtrConstant(1, DL));
    SDValue RHSlo = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, RHS,
                                DAG.getIntPtrConstant(0, DL));
    SDValue RHShi = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, RHS,
                                DAG.getIntPtrConstant(1, DL));
    Cmp = DAG.getNode(I8085ISD::CMP, DL, MVT::Glue, LHSlo, RHSlo);
    Cmp = DAG.getNode(I8085ISD::CMPC, DL, MVT::Glue, LHShi, RHShi, Cmp);
  } else {
    // Generate ordinary 16-bit comparison.
    Cmp = DAG.getNode(I8085ISD::CMP, DL, MVT::Glue, LHS, RHS);
  }

  return Cmp;
}

/// Returns appropriate I8085 CMP/CMPC nodes and corresponding condition code for
/// the given operands.
SDValue I8085TargetLowering::getI8085Cmp(SDValue LHS, SDValue RHS, ISD::CondCode CC,
                                     SDValue &I8085cc, SelectionDAG &DAG,
                                     SDLoc DL) const {
  SDValue Cmp;
  EVT VT = LHS.getValueType();
  bool UseTest = false;

  switch (CC) {
  default:
    break;
  case ISD::SETLE: {
    // Swap operands and reverse the branching condition.
    std::swap(LHS, RHS);
    CC = ISD::SETGE;
    break;
  }
  case ISD::SETGT: {
    if (const ConstantSDNode *C = dyn_cast<ConstantSDNode>(RHS)) {
      switch (C->getSExtValue()) {
      case -1: {
        // When doing lhs > -1 use a tst instruction on the top part of lhs
        // and use brpl instead of using a chain of cp/cpc.
        UseTest = true;
        I8085cc = DAG.getConstant(I8085CC::COND_PL, DL, MVT::i8);
        break;
      }
      case 0: {
        // Turn lhs > 0 into 0 < lhs since 0 can be materialized with
        // __zero_reg__ in lhs.
        RHS = LHS;
        LHS = DAG.getConstant(0, DL, VT);
        CC = ISD::SETLT;
        break;
      }
      default: {
        // Turn lhs < rhs with lhs constant into rhs >= lhs+1, this allows
        // us to  fold the constant into the cmp instruction.
        RHS = DAG.getConstant(C->getSExtValue() + 1, DL, VT);
        CC = ISD::SETGE;
        break;
      }
      }
      break;
    }
    // Swap operands and reverse the branching condition.
    std::swap(LHS, RHS);
    CC = ISD::SETLT;
    break;
  }
  case ISD::SETLT: {
    if (const ConstantSDNode *C = dyn_cast<ConstantSDNode>(RHS)) {
      switch (C->getSExtValue()) {
      case 1: {
        // Turn lhs < 1 into 0 >= lhs since 0 can be materialized with
        // __zero_reg__ in lhs.
        RHS = LHS;
        LHS = DAG.getConstant(0, DL, VT);
        CC = ISD::SETGE;
        break;
      }
      case 0: {
        // When doing lhs < 0 use a tst instruction on the top part of lhs
        // and use brmi instead of using a chain of cp/cpc.
        UseTest = true;
        I8085cc = DAG.getConstant(I8085CC::COND_MI, DL, MVT::i8);
        break;
      }
      }
    }
    break;
  }
  case ISD::SETULE: {
    // Swap operands and reverse the branching condition.
    std::swap(LHS, RHS);
    CC = ISD::SETUGE;
    break;
  }
  case ISD::SETUGT: {
    // Turn lhs < rhs with lhs constant into rhs >= lhs+1, this allows us to
    // fold the constant into the cmp instruction.
    if (const ConstantSDNode *C = dyn_cast<ConstantSDNode>(RHS)) {
      RHS = DAG.getConstant(C->getSExtValue() + 1, DL, VT);
      CC = ISD::SETUGE;
      break;
    }
    // Swap operands and reverse the branching condition.
    std::swap(LHS, RHS);
    CC = ISD::SETULT;
    break;
  }
  }

  // Expand 32 and 64 bit comparisons with custom CMP and CMPC nodes instead of
  // using the default and/or/xor expansion code which is much longer.
  if (VT == MVT::i32) {
    SDValue LHSlo = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, LHS,
                                DAG.getIntPtrConstant(0, DL));
    SDValue LHShi = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, LHS,
                                DAG.getIntPtrConstant(1, DL));
    SDValue RHSlo = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, RHS,
                                DAG.getIntPtrConstant(0, DL));
    SDValue RHShi = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, RHS,
                                DAG.getIntPtrConstant(1, DL));

    if (UseTest) {
      // When using tst we only care about the highest part.
      SDValue Top = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, LHShi,
                                DAG.getIntPtrConstant(1, DL));
      Cmp = DAG.getNode(I8085ISD::TST, DL, MVT::Glue, Top);
    } else {
      Cmp = getI8085Cmp(LHSlo, RHSlo, DAG, DL);
      Cmp = DAG.getNode(I8085ISD::CMPC, DL, MVT::Glue, LHShi, RHShi, Cmp);
    }
  } else if (VT == MVT::i64) {
    SDValue LHS_0 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, LHS,
                                DAG.getIntPtrConstant(0, DL));
    SDValue LHS_1 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, LHS,
                                DAG.getIntPtrConstant(1, DL));

    SDValue LHS0 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, LHS_0,
                               DAG.getIntPtrConstant(0, DL));
    SDValue LHS1 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, LHS_0,
                               DAG.getIntPtrConstant(1, DL));
    SDValue LHS2 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, LHS_1,
                               DAG.getIntPtrConstant(0, DL));
    SDValue LHS3 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, LHS_1,
                               DAG.getIntPtrConstant(1, DL));

    SDValue RHS_0 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, RHS,
                                DAG.getIntPtrConstant(0, DL));
    SDValue RHS_1 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i32, RHS,
                                DAG.getIntPtrConstant(1, DL));

    SDValue RHS0 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, RHS_0,
                               DAG.getIntPtrConstant(0, DL));
    SDValue RHS1 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, RHS_0,
                               DAG.getIntPtrConstant(1, DL));
    SDValue RHS2 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, RHS_1,
                               DAG.getIntPtrConstant(0, DL));
    SDValue RHS3 = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i16, RHS_1,
                               DAG.getIntPtrConstant(1, DL));

    if (UseTest) {
      // When using tst we only care about the highest part.
      SDValue Top = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, LHS3,
                                DAG.getIntPtrConstant(1, DL));
      Cmp = DAG.getNode(I8085ISD::TST, DL, MVT::Glue, Top);
    } else {
      Cmp = getI8085Cmp(LHS0, RHS0, DAG, DL);
      Cmp = DAG.getNode(I8085ISD::CMPC, DL, MVT::Glue, LHS1, RHS1, Cmp);
      Cmp = DAG.getNode(I8085ISD::CMPC, DL, MVT::Glue, LHS2, RHS2, Cmp);
      Cmp = DAG.getNode(I8085ISD::CMPC, DL, MVT::Glue, LHS3, RHS3, Cmp);
    }
  } else if (VT == MVT::i8 || VT == MVT::i16) {
    if (UseTest) {
      // When using tst we only care about the highest part.
      Cmp = DAG.getNode(I8085ISD::TST, DL, MVT::Glue,
                        (VT == MVT::i8)
                            ? LHS
                            : DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8,
                                          LHS, DAG.getIntPtrConstant(1, DL)));
    } else {
      Cmp = getI8085Cmp(LHS, RHS, DAG, DL);
    }
  } else {
    llvm_unreachable("Invalid comparison size");
  }

  // When using a test instruction I8085cc is already set.
  if (!UseTest) {
    I8085cc = DAG.getConstant(intCCToI8085CC(CC), DL, MVT::i8);
  }

  return Cmp;
}


SDValue I8085TargetLowering::LowerStore(SDValue Op,
                                              SelectionDAG &DAG) const {
                                                
  std::cout << "********************************" << "\n";
  return Op;
}


SDValue I8085TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("Don't know how to custom lower this!");

  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
  case ISD::STORE:
    return LowerStore(Op, DAG);  
  }
  return SDValue();
}

/// Replace a node with an illegal result type
/// with a new node built out of custom code.
void I8085TargetLowering::ReplaceNodeResults(SDNode *N,
                                           SmallVectorImpl<SDValue> &Results,
                                           SelectionDAG &DAG) const {
  SDLoc DL(N);

  switch (N->getOpcode()) {
  case ISD::ADD: {
    // Convert add (x, imm) into sub (x, -imm).
    if (const ConstantSDNode *C = dyn_cast<ConstantSDNode>(N->getOperand(1))) {
      SDValue Sub = DAG.getNode(
          ISD::SUB, DL, N->getValueType(0), N->getOperand(0),
          DAG.getConstant(-C->getAPIntValue(), DL, C->getValueType(0)));
      Results.push_back(Sub);
    }
    break;
  }
  default: {
    SDValue Res = LowerOperation(SDValue(N, 0), DAG);

    for (unsigned I = 0, E = Res->getNumValues(); I != E; ++I)
      Results.push_back(Res.getValue(I));

    break;
  }
  }
}

/// Return true if the addressing mode represented
/// by AM is legal for this target, for a load/store of the specified type.
bool I8085TargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                              const AddrMode &AM, Type *Ty,
                                              unsigned AS,
                                              Instruction *I) const {
  int64_t Offs = AM.BaseOffs;

  // Allow absolute addresses.
  if (AM.BaseGV && !AM.HasBaseReg && AM.Scale == 0 && Offs == 0) {
    return true;
  }

  // Flash memory instructions only allow zero offsets.
  if (isa<PointerType>(Ty) && AS == I8085::ProgramMemory) {
    return false;
  }

  // Allow reg+<6bit> offset.
  if (Offs < 0)
    Offs = -Offs;
  if (AM.BaseGV == nullptr && AM.HasBaseReg && AM.Scale == 0 &&
      isUInt<6>(Offs)) {
    return true;
  }

  return false;
}

/// Returns true by value, base pointer and
/// offset pointer and addressing mode by reference if the node's address
/// can be legally represented as pre-indexed load / store address.
bool I8085TargetLowering::getPreIndexedAddressParts(SDNode *N, SDValue &Base,
                                                  SDValue &Offset,
                                                  ISD::MemIndexedMode &AM,
                                                  SelectionDAG &DAG) const {
  EVT VT;
  const SDNode *Op;
  SDLoc DL(N);

  if (const LoadSDNode *LD = dyn_cast<LoadSDNode>(N)) {
    VT = LD->getMemoryVT();
    Op = LD->getBasePtr().getNode();
    if (LD->getExtensionType() != ISD::NON_EXTLOAD)
      return false;
    if (I8085::isProgramMemoryAccess(LD)) {
      return false;
    }
  } else if (const StoreSDNode *ST = dyn_cast<StoreSDNode>(N)) {
    VT = ST->getMemoryVT();
    Op = ST->getBasePtr().getNode();
    if (I8085::isProgramMemoryAccess(ST)) {
      return false;
    }
  } else {
    return false;
  }

  if (VT != MVT::i8 && VT != MVT::i16) {
    return false;
  }

  if (Op->getOpcode() != ISD::ADD && Op->getOpcode() != ISD::SUB) {
    return false;
  }

  if (const ConstantSDNode *RHS = dyn_cast<ConstantSDNode>(Op->getOperand(1))) {
    int RHSC = RHS->getSExtValue();
    if (Op->getOpcode() == ISD::SUB)
      RHSC = -RHSC;

    if ((VT == MVT::i16 && RHSC != -2) || (VT == MVT::i8 && RHSC != -1)) {
      return false;
    }

    Base = Op->getOperand(0);
    Offset = DAG.getConstant(RHSC, DL, MVT::i8);
    AM = ISD::PRE_DEC;

    return true;
  }

  return false;
}

/// Returns true by value, base pointer and
/// offset pointer and addressing mode by reference if this node can be
/// combined with a load / store to form a post-indexed load / store.
bool I8085TargetLowering::getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                                   SDValue &Base,
                                                   SDValue &Offset,
                                                   ISD::MemIndexedMode &AM,
                                                   SelectionDAG &DAG) const {
  EVT VT;
  SDLoc DL(N);

  if (const LoadSDNode *LD = dyn_cast<LoadSDNode>(N)) {
    VT = LD->getMemoryVT();
    if (LD->getExtensionType() != ISD::NON_EXTLOAD)
      return false;
  } else if (const StoreSDNode *ST = dyn_cast<StoreSDNode>(N)) {
    VT = ST->getMemoryVT();
    if (I8085::isProgramMemoryAccess(ST)) {
      return false;
    }
  } else {
    return false;
  }

  if (VT != MVT::i8 && VT != MVT::i16) {
    return false;
  }

  if (Op->getOpcode() != ISD::ADD && Op->getOpcode() != ISD::SUB) {
    return false;
  }

  if (const ConstantSDNode *RHS = dyn_cast<ConstantSDNode>(Op->getOperand(1))) {
    int RHSC = RHS->getSExtValue();
    if (Op->getOpcode() == ISD::SUB)
      RHSC = -RHSC;
    if ((VT == MVT::i16 && RHSC != 2) || (VT == MVT::i8 && RHSC != 1)) {
      return false;
    }

    Base = Op->getOperand(0);
    Offset = DAG.getConstant(RHSC, DL, MVT::i8);
    AM = ISD::POST_INC;

    return true;
  }

  return false;
}

bool I8085TargetLowering::isOffsetFoldingLegal(
    const GlobalAddressSDNode *GA) const {
  return true;
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "I8085GenCallingConv.inc"

/// Registers for calling conventions, ordered in reverse as required by ABI.
/// Both arrays must be of the same length.
static const MCPhysReg RegList8I8085[] = {
    I8085::R25, I8085::R24, I8085::R23, I8085::R22, I8085::R21, I8085::R20,
    I8085::R19, I8085::R18, I8085::R17, I8085::R16, I8085::R15, I8085::R14,
    I8085::R13, I8085::R12, I8085::R11, I8085::R10, I8085::R9,  I8085::R8};
static const MCPhysReg RegList8Tiny[] = {I8085::R25, I8085::R24, I8085::R23,
                                         I8085::R22, I8085::R21, I8085::R20};
static const MCPhysReg RegList16I8085[] = {
    I8085::R26R25, I8085::R25R24, I8085::R24R23, I8085::R23R22, I8085::R22R21,
    I8085::R21R20, I8085::R20R19, I8085::R19R18, I8085::R18R17, I8085::R17R16,
    I8085::R16R15, I8085::R15R14, I8085::R14R13, I8085::R13R12, I8085::R12R11,
    I8085::R11R10, I8085::R10R9,  I8085::R9R8};
static const MCPhysReg RegList16Tiny[] = {I8085::R26R25, I8085::R25R24,
                                          I8085::R24R23, I8085::R23R22,
                                          I8085::R22R21, I8085::R21R20};

static_assert(array_lengthof(RegList8I8085) == array_lengthof(RegList16I8085),
              "8-bit and 16-bit register arrays must be of equal length");
static_assert(array_lengthof(RegList8Tiny) == array_lengthof(RegList16Tiny),
              "8-bit and 16-bit register arrays must be of equal length");

/// Analyze incoming and outgoing function arguments. We need custom C++ code
/// to handle special constraints in the ABI.
/// In addition, all pieces of a certain argument have to be passed either
/// using registers or the stack but never mixing both.
template <typename ArgT>
static void analyzeArguments(TargetLowering::CallLoweringInfo *CLI,
                             const Function *F, const DataLayout *TD,
                             const SmallVectorImpl<ArgT> &Args,
                             SmallVectorImpl<CCValAssign> &ArgLocs,
                             CCState &CCInfo, bool Tiny) {
  // Choose the proper register list for argument passing according to the ABI.
  ArrayRef<MCPhysReg> RegList8;
  ArrayRef<MCPhysReg> RegList16;
  if (Tiny) {
    RegList8 = makeArrayRef(RegList8Tiny, array_lengthof(RegList8Tiny));
    RegList16 = makeArrayRef(RegList16Tiny, array_lengthof(RegList16Tiny));
  } else {
    RegList8 = makeArrayRef(RegList8I8085, array_lengthof(RegList8I8085));
    RegList16 = makeArrayRef(RegList16I8085, array_lengthof(RegList16I8085));
  }

  unsigned NumArgs = Args.size();
  // This is the index of the last used register, in RegList*.
  // -1 means R26 (R26 is never actually used in CC).
  int RegLastIdx = -1;
  // Once a value is passed to the stack it will always be used
  bool UseStack = false;
  for (unsigned i = 0; i != NumArgs;) {
    MVT VT = Args[i].VT;
    // We have to count the number of bytes for each function argument, that is
    // those Args with the same OrigArgIndex. This is important in case the
    // function takes an aggregate type.
    // Current argument will be between [i..j).
    unsigned ArgIndex = Args[i].OrigArgIndex;
    unsigned TotalBytes = VT.getStoreSize();
    unsigned j = i + 1;
    for (; j != NumArgs; ++j) {
      if (Args[j].OrigArgIndex != ArgIndex)
        break;
      TotalBytes += Args[j].VT.getStoreSize();
    }
    // Round up to even number of bytes.
    TotalBytes = alignTo(TotalBytes, 2);
    // Skip zero sized arguments
    if (TotalBytes == 0)
      continue;
    // The index of the first register to be used
    unsigned RegIdx = RegLastIdx + TotalBytes;
    RegLastIdx = RegIdx;
    // If there are not enough registers, use the stack
    if (RegIdx >= RegList8.size()) {
      UseStack = true;
    }
    for (; i != j; ++i) {
      MVT VT = Args[i].VT;

      if (UseStack) {
        auto evt = EVT(VT).getTypeForEVT(CCInfo.getContext());
        unsigned Offset = CCInfo.AllocateStack(TD->getTypeAllocSize(evt),
                                               TD->getABITypeAlign(evt));
        CCInfo.addLoc(
            CCValAssign::getMem(i, VT, Offset, VT, CCValAssign::Full));
      } else {
        unsigned Reg;
        if (VT == MVT::i8) {
          Reg = CCInfo.AllocateReg(RegList8[RegIdx]);
        } else if (VT == MVT::i16) {
          Reg = CCInfo.AllocateReg(RegList16[RegIdx]);
        } else {
          llvm_unreachable(
              "calling convention can only manage i8 and i16 types");
        }
        assert(Reg && "register not available in calling convention");
        CCInfo.addLoc(CCValAssign::getReg(i, VT, Reg, VT, CCValAssign::Full));
        // Registers inside a particular argument are sorted in increasing order
        // (remember the array is reversed).
        RegIdx -= VT.getStoreSize();
      }
    }
  }
}

/// Count the total number of bytes needed to pass or return these arguments.
template <typename ArgT>
static unsigned
getTotalArgumentsSizeInBytes(const SmallVectorImpl<ArgT> &Args) {
  unsigned TotalBytes = 0;

  for (const ArgT &Arg : Args) {
    TotalBytes += Arg.VT.getStoreSize();
  }
  return TotalBytes;
}

/// Analyze incoming and outgoing value of returning from a function.
/// The algorithm is similar to analyzeArguments, but there can only be
/// one value, possibly an aggregate, and it is limited to 8 bytes.
template <typename ArgT>
static void analyzeReturnValues(const SmallVectorImpl<ArgT> &Args,
                                CCState &CCInfo, bool Tiny) {
  unsigned NumArgs = Args.size();
  unsigned TotalBytes = getTotalArgumentsSizeInBytes(Args);
  // CanLowerReturn() guarantees this assertion.
  assert(TotalBytes <= 8 &&
         "return values greater than 8 bytes cannot be lowered");

  // Choose the proper register list for argument passing according to the ABI.
  ArrayRef<MCPhysReg> RegList8;
  ArrayRef<MCPhysReg> RegList16;
  if (Tiny) {
    RegList8 = makeArrayRef(RegList8Tiny, array_lengthof(RegList8Tiny));
    RegList16 = makeArrayRef(RegList16Tiny, array_lengthof(RegList16Tiny));
  } else {
    RegList8 = makeArrayRef(RegList8I8085, array_lengthof(RegList8I8085));
    RegList16 = makeArrayRef(RegList16I8085, array_lengthof(RegList16I8085));
  }

  // GCC-ABI says that the size is rounded up to the next even number,
  // but actually once it is more than 4 it will always round up to 8.
  if (TotalBytes > 4) {
    TotalBytes = 8;
  } else {
    TotalBytes = alignTo(TotalBytes, 2);
  }

  // The index of the first register to use.
  int RegIdx = TotalBytes - 1;
  for (unsigned i = 0; i != NumArgs; ++i) {
    MVT VT = Args[i].VT;
    unsigned Reg;
    if (VT == MVT::i8) {
      Reg = CCInfo.AllocateReg(RegList8[RegIdx]);
    } else if (VT == MVT::i16) {
      Reg = CCInfo.AllocateReg(RegList16[RegIdx]);
    } else {
      llvm_unreachable("calling convention can only manage i8 and i16 types");
    }
    assert(Reg && "register not available in calling convention");
    CCInfo.addLoc(CCValAssign::getReg(i, VT, Reg, VT, CCValAssign::Full));
    // Registers sort in increasing order
    RegIdx -= VT.getStoreSize();
  }
}

SDValue I8085TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto DL = DAG.getDataLayout();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  // Variadic functions do not need all the analysis below.
  if (isVarArg) {
    CCInfo.AnalyzeFormalArguments(Ins, ArgCC_I8085_Vararg);
  } else {
    analyzeArguments(nullptr, &MF.getFunction(), &DL, Ins, ArgLocs, CCInfo,
                     Subtarget.hasTinyEncoding());
  }

  SDValue ArgValue;
  for (CCValAssign &VA : ArgLocs) {

    // Arguments stored on registers.
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();
      const TargetRegisterClass *RC;
      if (RegVT == MVT::i8) {
        RC = &I8085::GR8RegClass;
      } else if (RegVT == MVT::i16) {
        RC = &I8085::DREGSRegClass;
      } else {
        llvm_unreachable("Unknown argument type!");
      }

      Register Reg = MF.addLiveIn(VA.getLocReg(), RC);
      ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);

      // :NOTE: Clang should not promote any i8 into i16 but for safety the
      // following code will handle zexts or sexts generated by other
      // front ends. Otherwise:
      // If this is an 8 bit value, it is really passed promoted
      // to 16 bits. Insert an assert[sz]ext to capture this, then
      // truncate to the right size.
      switch (VA.getLocInfo()) {
      default:
        llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full:
        break;
      case CCValAssign::BCvt:
        ArgValue = DAG.getNode(ISD::BITCAST, dl, VA.getValVT(), ArgValue);
        break;
      case CCValAssign::SExt:
        ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
        break;
      case CCValAssign::ZExt:
        ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
        break;
      }

      InVals.push_back(ArgValue);
    } else {
      // Only arguments passed on the stack should make it here.
      assert(VA.isMemLoc());

      EVT LocVT = VA.getLocVT();

      // Create the frame index object for this incoming parameter.
      int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8,
                                     VA.getLocMemOffset(), true);

      // Create the SelectionDAG nodes corresponding to a load
      // from this parameter.
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DL));
      InVals.push_back(DAG.getLoad(LocVT, dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI)));
    }
  }

  // If the function takes variable number of arguments, make a frame index for
  // the start of the first vararg value... for expansion of llvm.va_start.
  if (isVarArg) {
    unsigned StackSize = CCInfo.getNextStackOffset();
    I8085MachineFunctionInfo *AFI = MF.getInfo<I8085MachineFunctionInfo>();

    AFI->setVarArgsFrameIndex(MFI.CreateFixedObject(2, StackSize, true));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue I8085TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &isTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();

  // I8085 does not yet support tail call optimization.
  isTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
  // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
  // node so that legalize doesn't hack it.
  const Function *F = nullptr;
  if (const GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    if (isa<Function>(GV))
      F = cast<Function>(GV);
    Callee =
        DAG.getTargetGlobalAddress(GV, DL, getPointerTy(DAG.getDataLayout()));
  } else if (const ExternalSymbolSDNode *ES =
                 dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(ES->getSymbol(),
                                         getPointerTy(DAG.getDataLayout()));
  }

  // Variadic functions do not need all the analysis below.
  if (isVarArg) {
    CCInfo.AnalyzeCallOperands(Outs, ArgCC_I8085_Vararg);
  } else {
    analyzeArguments(&CLI, F, &DAG.getDataLayout(), Outs, ArgLocs, CCInfo,
                     Subtarget.hasTinyEncoding());
  }

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;

  // First, walk the register assignments, inserting copies.
  unsigned AI, AE;
  bool HasStackArgs = false;
  for (AI = 0, AE = ArgLocs.size(); AI != AE; ++AI) {
    CCValAssign &VA = ArgLocs[AI];
    EVT RegVT = VA.getLocVT();
    SDValue Arg = OutVals[AI];

    // Promote the value if needed. With Clang this should not happen.
    switch (VA.getLocInfo()) {
    default:
      llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, RegVT, Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, RegVT, Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, DL, RegVT, Arg);
      break;
    case CCValAssign::BCvt:
      Arg = DAG.getNode(ISD::BITCAST, DL, RegVT, Arg);
      break;
    }

    // Stop when we encounter a stack argument, we need to process them
    // in reverse order in the loop below.
    if (VA.isMemLoc()) {
      HasStackArgs = true;
      break;
    }

    // Arguments that can be passed on registers must be kept in the RegsToPass
    // vector.
    RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
  }

  // Second, stack arguments have to walked.
  // Previously this code created chained stores but those chained stores appear
  // to be unchained in the legalization phase. Therefore, do not attempt to
  // chain them here. In fact, chaining them here somehow causes the first and
  // second store to be reversed which is the exact opposite of the intended
  // effect.
  if (HasStackArgs) {
    SmallVector<SDValue, 8> MemOpChains;
    for (; AI != AE; AI++) {
      CCValAssign &VA = ArgLocs[AI];
      SDValue Arg = OutVals[AI];

      assert(VA.isMemLoc());

      // SP points to one stack slot further so add one to adjust it.
      SDValue PtrOff = DAG.getNode(
          ISD::ADD, DL, getPointerTy(DAG.getDataLayout()),
          DAG.getRegister(I8085::SP, getPointerTy(DAG.getDataLayout())),
          DAG.getIntPtrConstant(VA.getLocMemOffset() + 1, DL));

      MemOpChains.push_back(
          DAG.getStore(Chain, DL, Arg, PtrOff,
                       MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
    }

    if (!MemOpChains.empty())
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);
  }

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emited instructions must be stuck together.
  SDValue InFlag;
  for (auto Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg.first, Reg.second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are known live
  // into the call.
  for (auto Reg : RegsToPass) {
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));
  }

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
  const uint32_t *Mask =
      TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);
  assert(Mask && "Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InFlag.getNode()) {
    Ops.push_back(InFlag);
  }

  Chain = DAG.getNode(I8085ISD::CALL, DL, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NumBytes, DL, true),
                             DAG.getIntPtrConstant(0, DL, true), InFlag, DL);

  if (!Ins.empty()) {
    InFlag = Chain.getValue(1);
  }

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, DL, DAG,
                         InVals);
}

/// Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue I8085TargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Handle runtime calling convs.
  if (CallConv == CallingConv::I8085_BUILTIN) {
    CCInfo.AnalyzeCallResult(Ins, RetCC_I8085_BUILTIN);
  } else {
    analyzeReturnValues(Ins, CCInfo, Subtarget.hasTinyEncoding());
  }

  // Copy all of the result registers out of their specified physreg.
  for (CCValAssign const &RVLoc : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLoc.getLocReg(), RVLoc.getValVT(),
                               InFlag)
                .getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

bool I8085TargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  if (CallConv == CallingConv::I8085_BUILTIN) {
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
    return CCInfo.CheckReturn(Outs, RetCC_I8085_BUILTIN);
  }

  unsigned TotalBytes = getTotalArgumentsSizeInBytes(Outs);
  return TotalBytes <= 8;
}

SDValue
I8085TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &dl, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of the return value to locations.
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_I8085_BUILTIN);

  // if (CallConv == CallingConv::I8085_BUILTIN) {
  //   CCInfo.AnalyzeReturn(Outs, RetCC_I8085_BUILTIN);
  // } else {
  //   analyzeReturnValues(Outs, CCInfo, Subtarget.hasTinyEncoding());
  // }

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);
  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    // assert(VA.isRegLoc() && "Can only return in registers!");
    
    if(VA.isRegLoc()){
    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);
        // Guarantee that all emitted copies are stuck together with flags.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    }

  }

  // Don't emit the ret/reti instruction when the naked attribute is present in
  // the function being compiled.
  if (MF.getFunction().getAttributes().hasFnAttr(Attribute::Naked)) {
    return Chain;
  }

  const I8085MachineFunctionInfo *AFI = MF.getInfo<I8085MachineFunctionInfo>();

  unsigned RetOpc = I8085ISD::RET_FLAG;

  RetOps[0] = Chain; // Update chain.

  if (Flag.getNode()) {
    RetOps.push_back(Flag);
  }

  return DAG.getNode(RetOpc, dl, MVT::Other, RetOps);
}

//===----------------------------------------------------------------------===//
//  Custom Inserters
//===----------------------------------------------------------------------===//

MachineBasicBlock *I8085TargetLowering::insertShift(MachineInstr &MI,
                                                  MachineBasicBlock *BB) const {
  unsigned Opc;
  const TargetRegisterClass *RC;
  bool HasRepeatedOperand = false;
  MachineFunction *F = BB->getParent();
  MachineRegisterInfo &RI = F->getRegInfo();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  DebugLoc dl = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Invalid shift opcode!");
  case I8085::Lsl8:
    Opc = I8085::ADDRdRr; // LSL is an alias of ADD Rd, Rd
    RC = &I8085::GPR8RegClass;
    HasRepeatedOperand = true;
    break;
  case I8085::Lsl16:
    Opc = I8085::LSLWRd;
    RC = &I8085::DREGSRegClass;
    break;
  case I8085::Asr8:
    Opc = I8085::ASRRd;
    RC = &I8085::GPR8RegClass;
    break;
  case I8085::Asr16:
    Opc = I8085::ASRWRd;
    RC = &I8085::DREGSRegClass;
    break;
  case I8085::Lsr8:
    Opc = I8085::LSRRd;
    RC = &I8085::GPR8RegClass;
    break;
  case I8085::Lsr16:
    Opc = I8085::LSRWRd;
    RC = &I8085::DREGSRegClass;
    break;
  case I8085::Rol8:
    Opc = I8085::ROLBRd;
    RC = &I8085::GPR8RegClass;
    break;
  case I8085::Rol16:
    Opc = I8085::ROLWRd;
    RC = &I8085::DREGSRegClass;
    break;
  case I8085::Ror8:
    Opc = I8085::RORBRd;
    RC = &I8085::GPR8RegClass;
    break;
  case I8085::Ror16:
    Opc = I8085::RORWRd;
    RC = &I8085::DREGSRegClass;
    break;
  }

  const BasicBlock *LLVM_BB = BB->getBasicBlock();

  MachineFunction::iterator I;
  for (I = BB->getIterator(); I != F->end() && &(*I) != BB; ++I)
    ;
  if (I != F->end())
    ++I;

  // Create loop block.
  MachineBasicBlock *LoopBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *CheckBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *RemBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, LoopBB);
  F->insert(I, CheckBB);
  F->insert(I, RemBB);

  // Update machine-CFG edges by transferring all successors of the current
  // block to the block containing instructions after shift.
  RemBB->splice(RemBB->begin(), BB, std::next(MachineBasicBlock::iterator(MI)),
                BB->end());
  RemBB->transferSuccessorsAndUpdatePHIs(BB);

  // Add edges BB => LoopBB => CheckBB => RemBB, CheckBB => LoopBB.
  BB->addSuccessor(CheckBB);
  LoopBB->addSuccessor(CheckBB);
  CheckBB->addSuccessor(LoopBB);
  CheckBB->addSuccessor(RemBB);

  Register ShiftAmtReg = RI.createVirtualRegister(&I8085::GPR8RegClass);
  Register ShiftAmtReg2 = RI.createVirtualRegister(&I8085::GPR8RegClass);
  Register ShiftReg = RI.createVirtualRegister(RC);
  Register ShiftReg2 = RI.createVirtualRegister(RC);
  Register ShiftAmtSrcReg = MI.getOperand(2).getReg();
  Register SrcReg = MI.getOperand(1).getReg();
  Register DstReg = MI.getOperand(0).getReg();

  // BB:
  // rjmp CheckBB
  BuildMI(BB, dl, TII.get(I8085::RJMPk)).addMBB(CheckBB);

  // LoopBB:
  // ShiftReg2 = shift ShiftReg
  auto ShiftMI = BuildMI(LoopBB, dl, TII.get(Opc), ShiftReg2).addReg(ShiftReg);
  if (HasRepeatedOperand)
    ShiftMI.addReg(ShiftReg);

  // CheckBB:
  // ShiftReg = phi [%SrcReg, BB], [%ShiftReg2, LoopBB]
  // ShiftAmt = phi [%N,      BB], [%ShiftAmt2, LoopBB]
  // DestReg  = phi [%SrcReg, BB], [%ShiftReg,  LoopBB]
  // ShiftAmt2 = ShiftAmt - 1;
  // if (ShiftAmt2 >= 0) goto LoopBB;
  BuildMI(CheckBB, dl, TII.get(I8085::PHI), ShiftReg)
      .addReg(SrcReg)
      .addMBB(BB)
      .addReg(ShiftReg2)
      .addMBB(LoopBB);
  BuildMI(CheckBB, dl, TII.get(I8085::PHI), ShiftAmtReg)
      .addReg(ShiftAmtSrcReg)
      .addMBB(BB)
      .addReg(ShiftAmtReg2)
      .addMBB(LoopBB);
  BuildMI(CheckBB, dl, TII.get(I8085::PHI), DstReg)
      .addReg(SrcReg)
      .addMBB(BB)
      .addReg(ShiftReg2)
      .addMBB(LoopBB);

  BuildMI(CheckBB, dl, TII.get(I8085::DECRd), ShiftAmtReg2).addReg(ShiftAmtReg);
  BuildMI(CheckBB, dl, TII.get(I8085::BRPLk)).addMBB(LoopBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return RemBB;
}

static bool isCopyMulResult(MachineBasicBlock::iterator const &I) {
  if (I->getOpcode() == I8085::COPY) {
    Register SrcReg = I->getOperand(1).getReg();
    return (SrcReg == I8085::R0 || SrcReg == I8085::R1);
  }

  return false;
}

// The mul instructions wreak havock on our zero_reg R1. We need to clear it
// after the result has been evacuated. This is probably not the best way to do
// it, but it works for now.
MachineBasicBlock *I8085TargetLowering::insertMul(MachineInstr &MI,
                                                MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  MachineBasicBlock::iterator I(MI);
  ++I; // in any case insert *after* the mul instruction
  if (isCopyMulResult(I))
    ++I;
  if (isCopyMulResult(I))
    ++I;
  BuildMI(*BB, I, MI.getDebugLoc(), TII.get(I8085::EORRdRr), I8085::R1)
      .addReg(I8085::R1)
      .addReg(I8085::R1);
  return BB;
}

// Insert a read from R1, which almost always contains the value 0.
MachineBasicBlock *
I8085TargetLowering::insertCopyR1(MachineInstr &MI, MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  MachineBasicBlock::iterator I(MI);
  BuildMI(*BB, I, MI.getDebugLoc(), TII.get(I8085::COPY))
      .add(MI.getOperand(0))
      .addReg(I8085::R1);
  MI.eraseFromParent();
  return BB;
}

// Lower atomicrmw operation to disable interrupts, do operation, and restore
// interrupts. This works because all I8085 microcontrollers are single core.
MachineBasicBlock *I8085TargetLowering::insertAtomicArithmeticOp(
    MachineInstr &MI, MachineBasicBlock *BB, unsigned Opcode, int Width) const {
  MachineRegisterInfo &MRI = BB->getParent()->getRegInfo();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  MachineBasicBlock::iterator I(MI);
  const Register SCRATCH_REGISTER = I8085::R0;
  DebugLoc dl = MI.getDebugLoc();

  // Example instruction sequence, for an atomic 8-bit add:
  //   ldi r25, 5
  //   in r0, SREG
  //   cli
  //   ld r24, X
  //   add r25, r24
  //   st X, r25
  //   out SREG, r0

  const TargetRegisterClass *RC =
      (Width == 8) ? &I8085::GPR8RegClass : &I8085::DREGSRegClass;
  unsigned LoadOpcode = (Width == 8) ? I8085::LDRdPtr : I8085::LDWRdPtr;
  unsigned StoreOpcode = (Width == 8) ? I8085::STPtrRr : I8085::STWPtrRr;

  // Disable interrupts.
  BuildMI(*BB, I, dl, TII.get(I8085::INRdA), SCRATCH_REGISTER)
      .addImm(Subtarget.getIORegSREG());
  BuildMI(*BB, I, dl, TII.get(I8085::BCLRs)).addImm(7);

  // Load the original value.
  BuildMI(*BB, I, dl, TII.get(LoadOpcode), MI.getOperand(0).getReg())
      .add(MI.getOperand(1));

  // Do the arithmetic operation.
  Register Result = MRI.createVirtualRegister(RC);
  BuildMI(*BB, I, dl, TII.get(Opcode), Result)
      .addReg(MI.getOperand(0).getReg())
      .add(MI.getOperand(2));

  // Store the result.
  BuildMI(*BB, I, dl, TII.get(StoreOpcode))
      .add(MI.getOperand(1))
      .addReg(Result);

  // Restore interrupts.
  BuildMI(*BB, I, dl, TII.get(I8085::OUTARr))
      .addImm(Subtarget.getIORegSREG())
      .addReg(SCRATCH_REGISTER);

  // Remove the pseudo instruction.
  MI.eraseFromParent();
  return BB;
}

MachineBasicBlock *
I8085TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *MBB) const {
  int Opc = MI.getOpcode();

  // Pseudo shift instructions with a non constant shift amount are expanded
  // into a loop.
  switch (Opc) {
  case I8085::Lsl8:
  case I8085::Lsl16:
  case I8085::Lsr8:
  case I8085::Lsr16:
  case I8085::Rol8:
  case I8085::Rol16:
  case I8085::Ror8:
  case I8085::Ror16:
  case I8085::Asr8:
  case I8085::Asr16:
    return insertShift(MI, MBB);
  case I8085::MULRdRr:
  case I8085::MULSRdRr:
    return insertMul(MI, MBB);
  case I8085::CopyR1:
    return insertCopyR1(MI, MBB);
  case I8085::AtomicLoadAdd8:
    return insertAtomicArithmeticOp(MI, MBB, I8085::ADDRdRr, 8);
  case I8085::AtomicLoadAdd16:
    return insertAtomicArithmeticOp(MI, MBB, I8085::ADDWRdRr, 16);
  case I8085::AtomicLoadSub8:
    return insertAtomicArithmeticOp(MI, MBB, I8085::SUBRdRr, 8);
  case I8085::AtomicLoadSub16:
    return insertAtomicArithmeticOp(MI, MBB, I8085::SUBWRdRr, 16);
  case I8085::AtomicLoadAnd8:
    return insertAtomicArithmeticOp(MI, MBB, I8085::ANDRdRr, 8);
  case I8085::AtomicLoadAnd16:
    return insertAtomicArithmeticOp(MI, MBB, I8085::ANDWRdRr, 16);
  case I8085::AtomicLoadOr8:
    return insertAtomicArithmeticOp(MI, MBB, I8085::ORRdRr, 8);
  case I8085::AtomicLoadOr16:
    return insertAtomicArithmeticOp(MI, MBB, I8085::ORWRdRr, 16);
  case I8085::AtomicLoadXor8:
    return insertAtomicArithmeticOp(MI, MBB, I8085::EORRdRr, 8);
  case I8085::AtomicLoadXor16:
    return insertAtomicArithmeticOp(MI, MBB, I8085::EORWRdRr, 16);
  }

  assert((Opc == I8085::Select16 || Opc == I8085::Select8) &&
         "Unexpected instr type to insert");

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
    BuildMI(MBB, dl, TII.get(I8085::RJMPk)).addMBB(FallThrough);
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

  I8085CC::CondCodes CC = (I8085CC::CondCodes)MI.getOperand(3).getImm();
  BuildMI(MBB, dl, TII.getBrCond(CC)).addMBB(trueMBB);
  BuildMI(MBB, dl, TII.get(I8085::RJMPk)).addMBB(falseMBB);
  MBB->addSuccessor(falseMBB);
  MBB->addSuccessor(trueMBB);

  // Unconditionally flow back to the true block
  BuildMI(falseMBB, dl, TII.get(I8085::RJMPk)).addMBB(trueMBB);
  falseMBB->addSuccessor(trueMBB);

  // Set up the Phi node to determine where we came from
  BuildMI(*trueMBB, trueMBB->begin(), dl, TII.get(I8085::PHI),
          MI.getOperand(0).getReg())
      .addReg(MI.getOperand(1).getReg())
      .addMBB(MBB)
      .addReg(MI.getOperand(2).getReg())
      .addMBB(falseMBB);

  MI.eraseFromParent(); // The pseudo instruction is gone now.
  return trueMBB;
}

//===----------------------------------------------------------------------===//
//  Inline Asm Support
//===----------------------------------------------------------------------===//

I8085TargetLowering::ConstraintType
I8085TargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    // See http://www.nongnu.org/I8085-libc/user-manual/inline_asm.html
    switch (Constraint[0]) {
    default:
      break;
    case 'a': // Simple upper registers
    case 'b': // Base pointer registers pairs
    case 'd': // Upper register
    case 'l': // Lower registers
    case 'e': // Pointer register pairs
    case 'q': // Stack pointer register
    case 'r': // Any register
    case 'w': // Special upper register pairs
      return C_RegisterClass;
    case 't': // Temporary register
    case 'x':
    case 'X': // Pointer register pair X
    case 'y':
    case 'Y': // Pointer register pair Y
    case 'z':
    case 'Z': // Pointer register pair Z
      return C_Register;
    case 'Q': // A memory address based on Y or Z pointer with displacement.
      return C_Memory;
    case 'G': // Floating point constant
    case 'I': // 6-bit positive integer constant
    case 'J': // 6-bit negative integer constant
    case 'K': // Integer constant (Range: 2)
    case 'L': // Integer constant (Range: 0)
    case 'M': // 8-bit integer constant
    case 'N': // Integer constant (Range: -1)
    case 'O': // Integer constant (Range: 8, 16, 24)
    case 'P': // Integer constant (Range: 1)
    case 'R': // Integer constant (Range: -6 to 5)x
      return C_Immediate;
    }
  }

  return TargetLowering::getConstraintType(Constraint);
}

unsigned
I8085TargetLowering::getInlineAsmMemConstraint(StringRef ConstraintCode) const {
  // Not sure if this is actually the right thing to do, but we got to do
  // *something* [agnat]
  switch (ConstraintCode[0]) {
  case 'Q':
    return InlineAsm::Constraint_Q;
  }
  return TargetLowering::getInlineAsmMemConstraint(ConstraintCode);
}

I8085TargetLowering::ConstraintWeight
I8085TargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {
  ConstraintWeight weight = CW_Invalid;
  Value *CallOperandVal = info.CallOperandVal;

  // If we don't have a value, we can't do a match,
  // but allow it at the lowest weight.
  // (this behaviour has been copied from the ARM backend)
  if (!CallOperandVal) {
    return CW_Default;
  }

  // Look at the constraint type.
  switch (*constraint) {
  default:
    weight = TargetLowering::getSingleConstraintMatchWeight(info, constraint);
    break;
  case 'd':
  case 'r':
  case 'l':
    weight = CW_Register;
    break;
  case 'a':
  case 'b':
  case 'e':
  case 'q':
  case 't':
  case 'w':
  case 'x':
  case 'X':
  case 'y':
  case 'Y':
  case 'z':
  case 'Z':
    weight = CW_SpecificReg;
    break;
  case 'G':
    if (const ConstantFP *C = dyn_cast<ConstantFP>(CallOperandVal)) {
      if (C->isZero()) {
        weight = CW_Constant;
      }
    }
    break;
  case 'I':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if (isUInt<6>(C->getZExtValue())) {
        weight = CW_Constant;
      }
    }
    break;
  case 'J':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if ((C->getSExtValue() >= -63) && (C->getSExtValue() <= 0)) {
        weight = CW_Constant;
      }
    }
    break;
  case 'K':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if (C->getZExtValue() == 2) {
        weight = CW_Constant;
      }
    }
    break;
  case 'L':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if (C->getZExtValue() == 0) {
        weight = CW_Constant;
      }
    }
    break;
  case 'M':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if (isUInt<8>(C->getZExtValue())) {
        weight = CW_Constant;
      }
    }
    break;
  case 'N':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if (C->getSExtValue() == -1) {
        weight = CW_Constant;
      }
    }
    break;
  case 'O':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if ((C->getZExtValue() == 8) || (C->getZExtValue() == 16) ||
          (C->getZExtValue() == 24)) {
        weight = CW_Constant;
      }
    }
    break;
  case 'P':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if (C->getZExtValue() == 1) {
        weight = CW_Constant;
      }
    }
    break;
  case 'R':
    if (const ConstantInt *C = dyn_cast<ConstantInt>(CallOperandVal)) {
      if ((C->getSExtValue() >= -6) && (C->getSExtValue() <= 5)) {
        weight = CW_Constant;
      }
    }
    break;
  case 'Q':
    weight = CW_Memory;
    break;
  }

  return weight;
}

std::pair<unsigned, const TargetRegisterClass *>
I8085TargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                StringRef Constraint,
                                                MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'a': // Simple upper registers r16..r23.
      if (VT == MVT::i8)
        return std::make_pair(0U, &I8085::LD8loRegClass);
      else if (VT == MVT::i16)
        return std::make_pair(0U, &I8085::DREGSLD8loRegClass);
      break;
    case 'b': // Base pointer registers: y, z.
      if (VT == MVT::i8 || VT == MVT::i16)
        return std::make_pair(0U, &I8085::PTRDISPREGSRegClass);
      break;
    case 'd': // Upper registers r16..r31.
      if (VT == MVT::i8)
        return std::make_pair(0U, &I8085::LD8RegClass);
      else if (VT == MVT::i16)
        return std::make_pair(0U, &I8085::DLDREGSRegClass);
      break;
    case 'l': // Lower registers r0..r15.
      if (VT == MVT::i8)
        return std::make_pair(0U, &I8085::GPR8loRegClass);
      else if (VT == MVT::i16)
        return std::make_pair(0U, &I8085::DREGSloRegClass);
      break;
    case 'e': // Pointer register pairs: x, y, z.
      if (VT == MVT::i8 || VT == MVT::i16)
        return std::make_pair(0U, &I8085::PTRREGSRegClass);
      break;
    case 'q': // Stack pointer register: SPH:SPL.
      return std::make_pair(0U, &I8085::GPRSPRegClass);
    case 'r': // Any register: r0..r31.
      if (VT == MVT::i8)
        return std::make_pair(0U, &I8085::GPR8RegClass);
      else if (VT == MVT::i16)
        return std::make_pair(0U, &I8085::DREGSRegClass);
      break;
    case 't': // Temporary register: r0.
      if (VT == MVT::i8)
        return std::make_pair(unsigned(I8085::R0), &I8085::GPR8RegClass);
      break;
    case 'w': // Special upper register pairs: r24, r26, r28, r30.
      if (VT == MVT::i8 || VT == MVT::i16)
        return std::make_pair(0U, &I8085::IWREGSRegClass);
      break;
    case 'x': // Pointer register pair X: r27:r26.
    case 'X':
      if (VT == MVT::i8 || VT == MVT::i16)
        return std::make_pair(unsigned(I8085::R27R26), &I8085::PTRREGSRegClass);
      break;
    case 'y': // Pointer register pair Y: r29:r28.
    case 'Y':
      if (VT == MVT::i8 || VT == MVT::i16)
        return std::make_pair(unsigned(I8085::R29R28), &I8085::PTRREGSRegClass);
      break;
    case 'z': // Pointer register pair Z: r31:r30.
    case 'Z':
      if (VT == MVT::i8 || VT == MVT::i16)
        return std::make_pair(unsigned(I8085::R31R30), &I8085::PTRREGSRegClass);
      break;
    default:
      break;
    }
  }

  return TargetLowering::getRegForInlineAsmConstraint(
      Subtarget.getRegisterInfo(), Constraint, VT);
}

void I8085TargetLowering::LowerAsmOperandForConstraint(SDValue Op,
                                                     std::string &Constraint,
                                                     std::vector<SDValue> &Ops,
                                                     SelectionDAG &DAG) const {
  SDValue Result;
  SDLoc DL(Op);
  EVT Ty = Op.getValueType();

  // Currently only support length 1 constraints.
  if (Constraint.length() != 1) {
    return;
  }

  char ConstraintLetter = Constraint[0];
  switch (ConstraintLetter) {
  default:
    break;
  // Deal with integers first:
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'R': {
    const ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op);
    if (!C) {
      return;
    }

    int64_t CVal64 = C->getSExtValue();
    uint64_t CUVal64 = C->getZExtValue();
    switch (ConstraintLetter) {
    case 'I': // 0..63
      if (!isUInt<6>(CUVal64))
        return;
      Result = DAG.getTargetConstant(CUVal64, DL, Ty);
      break;
    case 'J': // -63..0
      if (CVal64 < -63 || CVal64 > 0)
        return;
      Result = DAG.getTargetConstant(CVal64, DL, Ty);
      break;
    case 'K': // 2
      if (CUVal64 != 2)
        return;
      Result = DAG.getTargetConstant(CUVal64, DL, Ty);
      break;
    case 'L': // 0
      if (CUVal64 != 0)
        return;
      Result = DAG.getTargetConstant(CUVal64, DL, Ty);
      break;
    case 'M': // 0..255
      if (!isUInt<8>(CUVal64))
        return;
      // i8 type may be printed as a negative number,
      // e.g. 254 would be printed as -2,
      // so we force it to i16 at least.
      if (Ty.getSimpleVT() == MVT::i8) {
        Ty = MVT::i16;
      }
      Result = DAG.getTargetConstant(CUVal64, DL, Ty);
      break;
    case 'N': // -1
      if (CVal64 != -1)
        return;
      Result = DAG.getTargetConstant(CVal64, DL, Ty);
      break;
    case 'O': // 8, 16, 24
      if (CUVal64 != 8 && CUVal64 != 16 && CUVal64 != 24)
        return;
      Result = DAG.getTargetConstant(CUVal64, DL, Ty);
      break;
    case 'P': // 1
      if (CUVal64 != 1)
        return;
      Result = DAG.getTargetConstant(CUVal64, DL, Ty);
      break;
    case 'R': // -6..5
      if (CVal64 < -6 || CVal64 > 5)
        return;
      Result = DAG.getTargetConstant(CVal64, DL, Ty);
      break;
    }

    break;
  }
  case 'G':
    const ConstantFPSDNode *FC = dyn_cast<ConstantFPSDNode>(Op);
    if (!FC || !FC->isZero())
      return;
    // Soften float to i8 0
    Result = DAG.getTargetConstant(0, DL, MVT::i8);
    break;
  }

  if (Result.getNode()) {
    Ops.push_back(Result);
    return;
  }

  return TargetLowering::LowerAsmOperandForConstraint(Op, Constraint, Ops, DAG);
}

Register I8085TargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                              const MachineFunction &MF) const {
  Register Reg;

  if (VT == LLT::scalar(8)) {
    Reg = StringSwitch<unsigned>(RegName)
              .Case("r0", I8085::R0)
              .Case("r1", I8085::R1)
              .Default(0);
  } else {
    Reg = StringSwitch<unsigned>(RegName)
              .Case("r0", I8085::R1R0)
              .Case("sp", I8085::SP)
              .Default(0);
  }

  if (Reg)
    return Reg;

  report_fatal_error(
      Twine("Invalid register name \"" + StringRef(RegName) + "\"."));
}

} // end of namespace llvm
