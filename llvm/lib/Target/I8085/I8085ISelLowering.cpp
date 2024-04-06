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

I8085TargetLowering::I8085TargetLowering(const I8085TargetMachine &TM,
                                     const I8085Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  // Set up the register classes.
  addRegisterClass(MVT::i8, &I8085::GR8RegClass);
  addRegisterClass(MVT::i16, &I8085::GR16RegClass);
  addRegisterClass(MVT::i32, &I8085::GR32RegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(Subtarget.getRegisterInfo());

  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent);
  setSchedulingPreference(Sched::RegPressure);
  setStackPointerRegisterToSaveRestore(I8085::SP);
  setSupportsUnalignedAtomics(true);

  setTruncStoreAction(MVT::i16, MVT::i8, Expand);
  setTruncStoreAction(MVT::i32, MVT::i16, Expand);

  for (MVT VT : MVT::integer_valuetypes()) {
    for (auto N : {ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}) {
      setLoadExtAction(N, VT, MVT::i1, Promote);
      setLoadExtAction(N, VT, MVT::i8, Expand);
      setLoadExtAction(N, VT, MVT::i16, Expand);
      setLoadExtAction(N, VT, MVT::i32, Expand);
    }
  }

  setOperationAction(ISD::MUL, MVT::i8, LibCall);
  setOperationAction(ISD::MUL, MVT::i16, LibCall);
  setOperationAction(ISD::MUL, MVT::i32, LibCall);

  setOperationAction(ISD::SDIV, MVT::i8, LibCall);
  setOperationAction(ISD::SDIV, MVT::i16, LibCall);
  setOperationAction(ISD::SDIV, MVT::i32, LibCall);

  setOperationAction(ISD::SREM, MVT::i8, LibCall);
  setOperationAction(ISD::SREM, MVT::i16, LibCall);
  setOperationAction(ISD::SREM, MVT::i32, LibCall);

  setLibcallName(RTLIB::MUL_I8, "__mul8");
  setLibcallName(RTLIB::MUL_I16, "__mul16");
  setLibcallName(RTLIB::MUL_I32, "__mul32");

  setLibcallName(RTLIB::SDIV_I8, "__sdiv8");
  setLibcallName(RTLIB::SDIV_I16, "__sdiv16");
  setLibcallName(RTLIB::SDIV_I32, "__sdiv32");

  setLibcallName(RTLIB::SREM_I8, "__srem8");
  setLibcallName(RTLIB::SREM_I16, "__srem16");
  setLibcallName(RTLIB::SREM_I32, "__srem32");

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

SDValue I8085TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("Don't know how to custom lower this!");
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
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
static const MCPhysReg RegList8I8085[] = { I8085::B, I8085::C, I8085::D, I8085::E };

static const MCPhysReg RegList16I8085[] = { I8085::BC, I8085::DE };





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

    RegList8 = ArrayRef(RegList8I8085, std::size(RegList8I8085));
    RegList16 = ArrayRef(RegList16I8085, std::size(RegList16I8085));


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

    RegList8 = ArrayRef(RegList8I8085, std::size(RegList8I8085));
    RegList16 = ArrayRef(RegList16I8085, std::size(RegList16I8085));


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

  CCInfo.AnalyzeFormalArguments(Ins, ArgCC_I8085_Vararg);


  SDValue ArgValue;
  for (CCValAssign &VA : ArgLocs) {

    // Arguments stored on registers.
    if (VA.isRegLoc()) {
      llvm_unreachable("Args should be passed via Stack!");
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

      SDValue load=DAG.getLoad(LocVT, dl, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI));

      InVals.push_back(load);
    }
  }

  // If the function takes variable number of arguments, make a frame index for
  // the start of the first vararg value... for expansion of llvm.va_start.
  if (isVarArg) {
    unsigned StackSize = CCInfo.getStackSize();
    I8085MachineFunctionInfo *AFI = MF.getInfo<I8085MachineFunctionInfo>();

    AFI->setVarArgsFrameIndex(MFI.CreateFixedObject(2, StackSize, true));
  }

  return Chain;
}

uint8_t twos_complement(uint8_t val) { return -(unsigned int)val;}

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


  CCInfo.AnalyzeCallOperands(Outs, ArgCC_I8085_Vararg);

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

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getStackSize();


  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;

  // First, walk the register assignments, inserting copies.
  unsigned AI, AE;
  bool HasStackArgs = false;
  for (AI = 0, AE = ArgLocs.size(); AI != AE; ++AI) {
    CCValAssign &VA = ArgLocs[AI];
    EVT RegVT = VA.getLocVT();
    SDValue Arg = OutVals[AI];
    
    if (VA.isMemLoc()) {
      HasStackArgs = true;
      break;
    }

    // Arguments that can be passed on registers must be kept in the RegsToPass
    // vector.
    RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
  }

   Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);

  // Second, stack arguments have to walked.
  // Previously this code created chained stores but those chained stores appear
  // to be unchained in the legalization phase. Therefore, do not attempt to
  // chain them here. In fact, chaining them here somehow causes the first and
  // second store to be reversed which is the exact opposite of the intended
  // effect.
  MachineFrameInfo &MFI = MF.getFrameInfo();
  
  SDValue StackPtr;

  if (HasStackArgs) {
    SmallVector<SDValue, 8> MemOpChains;
    for (; AI != AE; AI++) {

    CCValAssign &VA = ArgLocs[AI];
    SDValue Arg = OutVals[AI];

    assert(VA.isMemLoc());

    // if(VA.getLocMemOffset()>0){
    //   SDValue PtrOff = DAG.getNode(
    //         ISD::ADD, DL, getPointerTy(DAG.getDataLayout()),
    //         DAG.getRegister(I8085::SP, getPointerTy(DAG.getDataLayout())),
    //         DAG.getIntPtrConstant(VA.getLocMemOffset(), DL));
    //   MemOpChains.push_back(
    //       DAG.getStore(Chain, DL, Arg, PtrOff,MachinePointerInfo()));
    // }
    // else{
      SDValue ptrConstant = DAG.getConstant(0,DL,MVT::i16);
      MemOpChains.push_back(
          DAG.getStore(Chain, DL, Arg, ptrConstant,MachinePointerInfo())
      );
    // }

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
  Chain = DAG.getCALLSEQ_END(Chain, NumBytes,0, InFlag, DL);

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

  CCInfo.AnalyzeCallResult(Ins, RetCC_I8085_BUILTIN);

  SmallVector<std::pair<int, unsigned>, 4> ResultMemLocs;

  // Copy all of the result registers out of their specified physreg.
  for (CCValAssign const &RVLoc : RVLocs) {
    if(RVLoc.isRegLoc()){
    Chain = DAG.getCopyFromReg(Chain, dl, RVLoc.getLocReg(), RVLoc.getValVT(),
                               InFlag)
                .getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
    }
    else{
      assert(RVLoc.isMemLoc() && "Must be memory location.");
      ResultMemLocs.push_back(
          std::make_pair(RVLoc.getLocMemOffset(), InVals.size()));
      // Reserve space for this result.
      InVals.push_back(SDValue());
    }
  }

    // Copy results out of memory.
  SmallVector<SDValue, 4> MemOpChains;
  for (unsigned i = 0, e = ResultMemLocs.size(); i != e; ++i) {
    int Offset = ResultMemLocs[i].first;
    unsigned Index = ResultMemLocs[i].second;
    
    SDValue ptrConstant = DAG.getConstant(Offset,dl,MVT::i16);
    SDValue Load = DAG.getLoad(MVT::i32, dl, Chain, ptrConstant, MachinePointerInfo());
    InVals[Index] = Load;
    MemOpChains.push_back(Load.getValue(1));
  }

  // Transform all loads nodes into one single node because
  // all load nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

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
  auto DL = DAG.getDataLayout();

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  MachineFunction &MF = DAG.getMachineFunction();

  CCInfo.AnalyzeReturn(Outs, RetCC_I8085_BUILTIN);

  MachineFrameInfo &MFI = MF.getFrameInfo();

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);
  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];
    if(VA.isRegLoc()){

      Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

      // Guarantee that all emitted copies are stuck together with flags.
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    }
    else{
    int Offset = VA.getLocMemOffset();
    unsigned ObjSize = VA.getLocVT().getStoreSize();
    // Create the frame index object for the memory location.
    int FI = MFI.CreateFixedObject(ObjSize, Offset, false);

    // Create a SelectionDAG node corresponding to a store
    // to this memory location.
    SDValue FIN = DAG.getFrameIndex(FI, MVT::i16);
    if(ObjSize==8) FIN = DAG.getFrameIndex(FI, MVT::i8);


    Chain = DAG.getStore(Chain, dl, OutVals[i], FIN,MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));


    unsigned RetOpc = I8085ISD::RET_FLAG;  
    RetOps[0] = Chain; 

    if (Flag.getNode()) {
          RetOps.push_back(Flag);
    }

    return DAG.getNode(RetOpc, dl, MVT::Other, RetOps); 
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

MachineBasicBlock *I8085TargetLowering::insertShiftSet(MachineInstr &MI,
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

  MachineBasicBlock *continuationMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *shiftLoopMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *checkMBB = MF->CreateMachineBasicBlock(LLVM_BB);

  MachineFunction::iterator I;
  for (I = MF->begin(); I != MF->end() && &(*I) != MBB; ++I)
    ;
  if (I != MF->end())
    ++I;
  
  MF->insert(I, shiftLoopMBB);
  MF->insert(I, checkMBB);
  MF->insert(I, continuationMBB);


  // Transfer remaining instructions and all successors of the current
  // block to the block which will contain the Phi node for the
  // select.
  continuationMBB->splice(continuationMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());

  continuationMBB->transferSuccessorsAndUpdatePHIs(MBB);

  MBB->addSuccessor(checkMBB);
  shiftLoopMBB->addSuccessor(checkMBB);
  checkMBB->addSuccessor(shiftLoopMBB);
  checkMBB->addSuccessor(continuationMBB);

  unsigned destReg = MI.getOperand(0).getReg();
  unsigned operandOne = MI.getOperand(1).getReg();
  unsigned operandTwo = MI.getOperand(2).getReg();

  unsigned counterTempReg = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));
  unsigned counterTempReg2 = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));

  unsigned tempHolderOne,tempHolderTwo;
  int rrOpcode;
  if(Opc==I8085::SHL_16 || Opc==I8085::SRA_16){
      rrOpcode = I8085::RR_16;
      if(Opc==I8085::SHL_16) rrOpcode=I8085::RL_16;
      tempHolderOne = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i16));
      tempHolderTwo = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i16));
  }

  if(Opc==I8085::SHL_8 || Opc==I8085::SRA_8){
      rrOpcode = I8085::RR_8;
      if(Opc==I8085::SHL_8) rrOpcode=I8085::RL_8;
      tempHolderOne = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));
      tempHolderTwo = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i8));
  }

  if(Opc==I8085::SHL_32 || Opc==I8085::SRA_32){
      rrOpcode = I8085::RR_32;
      if(Opc==I8085::SHL_32) rrOpcode=I8085::RL_32;
      tempHolderOne = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
      tempHolderTwo = MF->getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
  }

  //MBB:
  // Jump to loop MBB

  BuildMI(MBB, dl, TII.get(I8085::JMP))
     .addMBB(checkMBB);

  // LoopBB:
  // tempHolderTwo = shift tempHolderOne

  BuildMI(shiftLoopMBB, dl, TII.get(rrOpcode))
        .addReg(tempHolderTwo, RegState::Define)
        .addReg(tempHolderOne);

  // checkMBB:
  // tempHolderOne = phi [%operandOne, BB], [%tempHolderTwo, LoopBB]
  // counterTempReg2 = phi [%operandTwo, BB], [%counterTempReg, LoopBB]
  // destReg  = phi [%operandOne, BB], [%tempHolderTwo,  LoopBB]
  // counterTempReg = counterTempReg2 - 1;
  // if (counterTempReg >= 0) goto shiftLoopMBB;
  
  BuildMI(checkMBB, dl, TII.get(I8085::PHI), tempHolderOne)
      .addReg(operandOne)
      .addMBB(MBB)
      .addReg(tempHolderTwo)
      .addMBB(shiftLoopMBB);

  BuildMI(checkMBB, dl, TII.get(I8085::PHI), counterTempReg2)
      .addReg(operandTwo)
      .addMBB(MBB)
      .addReg(counterTempReg)
      .addMBB(shiftLoopMBB);

  BuildMI(checkMBB, dl, TII.get(I8085::PHI), destReg)
      .addReg(operandOne)
      .addMBB(MBB)
      .addReg(tempHolderTwo)
      .addMBB(shiftLoopMBB);

  BuildMI(checkMBB, dl, TII.get(I8085::DCR))
      .addReg(counterTempReg,RegState::Define)
      .addReg(counterTempReg2);

  BuildMI(checkMBB, dl, TII.get(I8085::JP))
            .addMBB(shiftLoopMBB);      

  MI.eraseFromParent();
  return continuationMBB;
}


MachineBasicBlock *I8085TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *MBB) const {
  int Opc = MI.getOpcode();

  // Pseudo shift instructions with a non constant shift amount are expanded
  // into a loop.
  switch (Opc) {
  case I8085::SET_NE_8:
  case I8085::SET_EQ_8:
  case I8085::SET_UGT_8:
  case I8085::SET_ULT_8:
  case I8085::SET_UGE_8:
  case I8085::SET_ULE_8:
    return insertCond8Set(MI, MBB);
  case I8085::SET_GT_8:
  case I8085::SET_LT_8:
  case I8085::SET_GE_8:
  case I8085::SET_LE_8:
    return insertSigned8Cond(MI,MBB); 
  case I8085::SET_DIFF_SIGN_GT_8:
  case I8085::SET_DIFF_SIGN_LT_8:
  case I8085::SET_DIFF_SIGN_GE_8:
  case I8085::SET_DIFF_SIGN_LE_8:
    return insertDifferentSigned8Cond(MI,MBB);

  case I8085::SET_NE_16:
  case I8085::SET_EQ_16:
    return insertEqualityCond16Set(MI,MBB);

  case I8085::SET_UGT_16:
  case I8085::SET_ULT_16:
  case I8085::SET_UGE_16:
  case I8085::SET_ULE_16:
    return insertCond16Set(MI, MBB); 

  case I8085::SET_GT_16:
  case I8085::SET_LT_16:
  case I8085::SET_GE_16:
  case I8085::SET_LE_16:
    return insertSignedCond16Set(MI, MBB);   

  case I8085::SET_DIFF_SIGN_LT_16:
  case I8085::SET_DIFF_SIGN_GT_16:
    return insertDifferentSignedCond16Set(MI, MBB); 
  

  case I8085::SET_NE_32:
  case I8085::SET_EQ_32:
    return insertEqualityCond32Set(MI,MBB);
  

  case I8085::SET_UGT_32:
  case I8085::SET_ULT_32:
  case I8085::SET_UGE_32:
  case I8085::SET_ULE_32:
    return insertCond32Set(MI, MBB); 
  
  case I8085::SET_GT_32:
  case I8085::SET_LT_32:
  case I8085::SET_GE_32:
  case I8085::SET_LE_32:
    return insertSignedCond32Set(MI, MBB); 


  case I8085::SET_DIFF_SIGN_LT_32:
  case I8085::SET_DIFF_SIGN_GT_32:
    return insertDifferentSignedCond32Set(MI, MBB);   

  case I8085::SHL_8:
  case I8085::SRA_8:
  case I8085::SHL_16:
  case I8085::SRA_16:
  case I8085::SHL_32:
  case I8085::SRA_32:
    return insertShiftSet(MI, MBB);      
  }

  assert( false &&
         "Unexpected instr type to insert");
  return MBB;
}


Register I8085TargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                              const MachineFunction &MF) const {
  Register Reg;

  if (VT == LLT::scalar(8)) {
    Reg = StringSwitch<unsigned>(RegName)
              .Default(0);
  } else {
    Reg = StringSwitch<unsigned>(RegName)
              .Default(0);
  }

  if (Reg)
    return Reg;

  report_fatal_error(
      Twine("Invalid register name \"" + StringRef(RegName) + "\"."));
}

} 