//===-- I8085InstrInfo.td - I8085 Instruction defs -------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the I8085 instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "I8085InstrFormats.td"

//===----------------------------------------------------------------------===//
// I8085 Type Profiles
//===----------------------------------------------------------------------===//

def SDT_I8085CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_I8085CallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_I8085Call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_I8085Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
def SDT_I8085Brcond
    : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>, SDTCisVT<1, i8>]>;
def SDT_I8085Cmp : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;
def SDT_I8085Tst : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def SDT_I8085SelectCC
    : SDTypeProfile<1, 3,
                    [SDTCisSameAs<0, 1>, SDTCisSameAs<1, 2>, SDTCisVT<3, i8>]>;

//===----------------------------------------------------------------------===//
// I8085 Specific Node Definitions
//===----------------------------------------------------------------------===//

def I8085retflag : SDNode<"I8085ISD::RET_FLAG", SDTNone,
                        [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def I8085retiflag : SDNode<"I8085ISD::RETI_FLAG", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def I8085callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_I8085CallSeqStart,
                              [SDNPHasChain, SDNPOutGlue]>;
def I8085callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_I8085CallSeqEnd,
                            [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def I8085call : SDNode<"I8085ISD::CALL", SDT_I8085Call,
                     [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def I8085Wrapper : SDNode<"I8085ISD::WRAPPER", SDT_I8085Wrapper>;

def I8085brcond
    : SDNode<"I8085ISD::BRCOND", SDT_I8085Brcond, [SDNPHasChain, SDNPInGlue]>;
def I8085cmp : SDNode<"I8085ISD::CMP", SDT_I8085Cmp, [SDNPOutGlue]>;
def I8085cmpc : SDNode<"I8085ISD::CMPC", SDT_I8085Cmp, [SDNPInGlue, SDNPOutGlue]>;
def I8085tst : SDNode<"I8085ISD::TST", SDT_I8085Tst, [SDNPOutGlue]>;
def I8085selectcc : SDNode<"I8085ISD::SELECT_CC", SDT_I8085SelectCC, [SDNPInGlue]>;

// Shift nodes.
def I8085lsl : SDNode<"I8085ISD::LSL", SDTIntUnaryOp>;
def I8085lsr : SDNode<"I8085ISD::LSR", SDTIntUnaryOp>;
def I8085rol : SDNode<"I8085ISD::ROL", SDTIntUnaryOp>;
def I8085ror : SDNode<"I8085ISD::ROR", SDTIntUnaryOp>;
def I8085asr : SDNode<"I8085ISD::ASR", SDTIntUnaryOp>;
def I8085lslhi : SDNode<"I8085ISD::LSLHI", SDTIntUnaryOp>;
def I8085lsrlo : SDNode<"I8085ISD::LSRLO", SDTIntUnaryOp>;
def I8085asrlo : SDNode<"I8085ISD::ASRLO", SDTIntUnaryOp>;
def I8085lslbn : SDNode<"I8085ISD::LSLBN", SDTIntBinOp>;
def I8085lsrbn : SDNode<"I8085ISD::LSRBN", SDTIntBinOp>;
def I8085asrbn : SDNode<"I8085ISD::ASRBN", SDTIntBinOp>;
def I8085lslwn : SDNode<"I8085ISD::LSLWN", SDTIntBinOp>;
def I8085lsrwn : SDNode<"I8085ISD::LSRWN", SDTIntBinOp>;
def I8085asrwn : SDNode<"I8085ISD::ASRWN", SDTIntBinOp>;

// Pseudo shift nodes for non-constant shift amounts.
def I8085lslLoop : SDNode<"I8085ISD::LSLLOOP", SDTIntShiftOp>;
def I8085lsrLoop : SDNode<"I8085ISD::LSRLOOP", SDTIntShiftOp>;
def I8085rolLoop : SDNode<"I8085ISD::ROLLOOP", SDTIntShiftOp>;
def I8085rorLoop : SDNode<"I8085ISD::RORLOOP", SDTIntShiftOp>;
def I8085asrLoop : SDNode<"I8085ISD::ASRLOOP", SDTIntShiftOp>;

// SWAP node.
def I8085Swap : SDNode<"I8085ISD::SWAP", SDTIntUnaryOp>;

//===----------------------------------------------------------------------===//
// I8085 Operands, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//


// imm_com8_XFORM - Return the complement of a imm_com8 value
def imm_com8_XFORM
    : SDNodeXForm<imm, [{
                    return CurDAG->getTargetConstant(
                        ~((uint8_t) N->getZExtValue()), SDLoc(N), MVT::i8);
                  }]>;

// imm_com8 - Match an immediate that is a complement
// of a 8-bit immediate.
// Note: this pattern doesn't require an encoder method and such, as it's
// only used on aliases (Pat<> and InstAlias<>). The actual encoding
// is handled by the destination instructions, which use imm_com8.
def imm_com8_asmoperand : AsmOperandClass { let Name = "ImmCom8"; }
def imm_com8 : Operand<i8> { let ParserMatchClass = imm_com8_asmoperand; }



def MemriAsmOperand : AsmOperandClass {
  let Name = "Memri";
  let ParserMethod = "parseMemriOperand";
}

/// Address operand for `reg+imm` used by STD and LDD.
def memri : Operand<iPTR> {
  let MIOperandInfo = (ops PTRDISPREGS, i16imm);

  let PrintMethod = "printMemri";
  let EncoderMethod = "encodeMemri";

  let ParserMatchClass = MemriAsmOperand;
}

// Address operand for `SP+imm` used by STD{W}SPQRr
def memspi : Operand<iPTR> { let MIOperandInfo = (ops GPRSP, i16imm); }

def relbrtarget_7 : Operand<OtherVT> {
  let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeRelCondBrTarget<I8085::fixup_7_pcrel>";
}

def brtarget_13 : Operand<OtherVT> {
  let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeRelCondBrTarget<I8085::fixup_13_pcrel>";
}

def rcalltarget_13 : Operand<i16> {
  let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeRelCondBrTarget<I8085::fixup_13_pcrel>";
}

// The target of a 22 or 16-bit call/jmp instruction.
def call_target : Operand<iPTR> {
  let EncoderMethod = "encodeCallTarget";
  let DecoderMethod = "decodeCallTarget";
}

// A 16-bit address (which can lead to an R_I8085_16 relocation).
def imm16 : Operand<i16> { let EncoderMethod = "encodeImm<I8085::fixup_16, 2>"; }

/// A 6-bit immediate used in the ADIW/SBIW instructions.
def imm_arith6 : Operand<i16> {
  let EncoderMethod = "encodeImm<I8085::fixup_6_adiw, 0>";
}



// Addressing mode pattern reg+imm6
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [], [SDNPWantRoot]>;

// AsmOperand class for a pointer register.
// Used with the LD/ST family of instructions.
// See FSTLD in I8085InstrFormats.td
def PtrRegAsmOperand : AsmOperandClass { let Name = "Reg"; }

// A special operand type for the LD/ST instructions.
// It converts the pointer register number into a two-bit field used in the
// instruction.
def LDSTPtrReg : Operand<i16> {
  let MIOperandInfo = (ops PTRREGS);
  let EncoderMethod = "encodeLDSTPtrReg";

  let ParserMatchClass = PtrRegAsmOperand;
}


//===----------------------------------------------------------------------===//
// I8085 predicates for subtarget features
//===----------------------------------------------------------------------===//

def HasSRAM : Predicate<"Subtarget->hasSRAM()">,
              AssemblerPredicate<(all_of FeatureSRAM)>;



//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// I8085 Instruction list
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

// ADJCALLSTACKDOWN/UP implicitly use/def SP because they may be expanded into
// a stack adjustment and the codegen must know that they may modify the stack
// pointer before prolog-epilog rewriting occurs.
// Pessimistically assume ADJCALLSTACKDOWN / ADJCALLSTACKUP will become
// sub / add which can clobber SREG.
let Defs = [SP], Uses = [SP] in {
  def ADJCALLSTACKDOWN : Pseudo<(outs),
                                (ins i16imm
                                 : $amt, i16imm
                                 : $amt2),
                                "#ADJCALLSTACKDOWN", [(I8085callseq_start timm
                                                       : $amt, timm
                                                       : $amt2)]>;

  // R31R30 is used to update SP. It is normally free because it is a
  // call-clobbered register but it is necessary to set it as a def as the
  // register allocator might use it in rare cases (for rematerialization, it
  // seems). hasSideEffects needs to be set to true so this instruction isn't
  // considered dead.
  def ADJCALLSTACKUP
      : Pseudo<(outs),
               (ins i16imm
                : $amt1, i16imm
                : $amt2),
               "#ADJCALLSTACKUP", [(I8085callseq_end timm
                                    : $amt1, timm
                                    : $amt2)]>;
}


// Indirect loads (with postincrement or predecrement).
let mayLoad = 1, hasSideEffects = 0,
    Constraints = "$ptrreg = $base_wb,@earlyclobber $reg" in {
  def LDRdPtrPi : FSTLD<0, 0b01,
                        (outs GPR8
                         : $reg, PTRREGS
                         : $base_wb),
                        (ins LDSTPtrReg
                         : $ptrreg),
                        "ld\t$reg, $ptrreg+", []>,
                  Requires<[HasSRAM]>;

  // LDW Rd+1:Rd, P+
  // Expands to:
  // ld Rd,   P+
  // ld Rd+1, P+
  def LDWRdPtrPi : Pseudo<(outs DREGS
                           : $reg, PTRREGS
                           : $base_wb),
                          (ins PTRREGS
                           : $ptrreg),
                          "ldw\t$reg, $ptrreg+", []>,
                   Requires<[HasSRAM]>;

  def LDRdPtrPd : FSTLD<0, 0b10,
                        (outs GPR8
                         : $reg, PTRREGS
                         : $base_wb),
                        (ins LDSTPtrReg
                         : $ptrreg),
                        "ld\t$reg, -$ptrreg", []>,
                  Requires<[HasSRAM]>;

  // LDW Rd+1:Rd, -P
  //
  // Expands to:
  // ld Rd+1, -P
  // ld Rd,   -P
  def LDWRdPtrPd : Pseudo<(outs DREGS
                           : $reg, PTRREGS
                           : $base_wb),
                          (ins PTRREGS
                           : $ptrreg),
                          "ldw\t$reg, -$ptrreg", []>,
                   Requires<[HasSRAM]>;
}

// Load indirect with displacement operations.
let canFoldAsLoad = 1, isReMaterializable = 1 in {
  let Constraints = "@earlyclobber $reg" in def LDDRdPtrQ
      : FSTDLDD<0,
                (outs GPR8
                 : $reg),
                (ins memri
                 : $memri),
                "ldd\t$reg, $memri", [(set i8
                                       : $reg, (load addr
                                                : $memri))]>,
      Requires<[HasSRAM]>;

  // LDDW Rd+1:Rd, P+q
  //
  // Expands to:
  // ldd Rd,   P+q
  // ldd Rd+1, P+q+1
  let Constraints = "@earlyclobber $dst" in def LDDWRdPtrQ
      : Pseudo<(outs DREGS
                : $dst),
               (ins memri
                : $memri),
               "lddw\t$dst, $memri", [(set i16
                                       : $dst, (load addr
                                                : $memri))]>,
      Requires<[HasSRAM]>;

  // An identical pseudo instruction to LDDWRdPtrQ, expect restricted to the Y
  // register and without the @earlyclobber flag.
  //
  // Used to work around a bug caused by the register allocator not
  // being able to handle the expansion of a COPY into an machine instruction
  // that has an earlyclobber flag. This is because the register allocator will
  // try expand a copy from a register slot into an earlyclobber instruction.
  // Instructions that are earlyclobber need to be in a dedicated earlyclobber
  // slot.
  //
  // This pseudo instruction can be used pre-I8085 pseudo expansion in order to
  // get a frame index load without directly using earlyclobber instructions.
  //
  // The pseudo expansion pass trivially expands this into LDDWRdPtrQ.
  //
  // This instruction may be removed once PR13375 is fixed.
  let mayLoad = 1,
      hasSideEffects = 0 in def LDDWRdYQ : Pseudo<(outs DREGS
                                                   : $dst),
                                                  (ins memri
                                                   : $memri),
                                                  "lddw\t$dst, $memri", []>,
      Requires<[HasSRAM]>;
}


// Indirect store from register to data space.
def STSKRr : F32DM<0b1, (outs),
                   (ins imm16
                    : $k, GPR8
                    : $rd),
                   "sts\t$k, $rd", [(store i8
                                     : $rd, imm
                                     : $k)]>,
             Requires<[HasSRAM]>;

// STSW K+1:K, Rr+1:Rr
//
// Expands to:
// sts Rr+1, (K+1:K) + 1
// sts Rr,   (K+1:K)
def STSWKRr : Pseudo<(outs),
                     (ins i16imm
                      : $dst, DREGS
                      : $src),
                     "stsw\t$dst, $src", [(store i16
                                           : $src, imm
                                           : $dst)]>,
              Requires<[HasSRAM]>;

// Indirect stores.
// ST P, Rr
// Stores the value of Rr into the location addressed by pointer P.
def STPtrRr : FSTLD<1, 0b00, (outs),
                    (ins LDSTPtrReg
                     : $ptrreg, GPR8
                     : $reg),
                    "st\t$ptrreg, $reg", [(store GPR8
                                           : $reg, i16
                                           : $ptrreg)]>,
              Requires<[HasSRAM]>;

// STW P, Rr+1:Rr
// Stores the value of Rr into the location addressed by pointer P.
//
// Expands to:
// st P, Rr
// std P+1, Rr+1
def STWPtrRr : Pseudo<(outs),
                      (ins PTRDISPREGS
                       : $ptrreg, DREGS
                       : $reg),
                      "stw\t$ptrreg, $reg", [(store i16
                                              : $reg, i16
                                              : $ptrreg)]>,
               Requires<[HasSRAM]>;

// Indirect stores (with postincrement or predecrement).
let Constraints = "$ptrreg = $base_wb,@earlyclobber $base_wb" in {

  // ST P+, Rr
  // Stores the value of Rr into the location addressed by pointer P.
  // Post increments P.
  def STPtrPiRr : FSTLD<1, 0b01,
                        (outs LDSTPtrReg
                         : $base_wb),
                        (ins LDSTPtrReg
                         : $ptrreg, GPR8
                         : $reg, i8imm
                         : $offs),
                        "st\t$ptrreg+, $reg", [(set i16
                                                : $base_wb, (post_store GPR8
                                                             : $reg, i16
                                                             : $ptrreg, imm
                                                             : $offs))]>,
                  Requires<[HasSRAM]>;

  // STW P+, Rr+1:Rr
  // Stores the value of Rr into the location addressed by pointer P.
  // Post increments P.
  //
  // Expands to:
  // st P+, Rr
  // st P+, Rr+1
  def STWPtrPiRr : Pseudo<(outs PTRREGS
                           : $base_wb),
                          (ins PTRREGS
                           : $ptrreg, DREGS
                           : $trh, i8imm
                           : $offs),
                          "stw\t$ptrreg+, $trh", [(set PTRREGS
                                                   : $base_wb, (post_store DREGS
                                                                : $trh, PTRREGS
                                                                : $ptrreg, imm
                                                                : $offs))]>,
                   Requires<[HasSRAM]>;

  // ST -P, Rr
  // Stores the value of Rr into the location addressed by pointer P.
  // Pre decrements P.
  def STPtrPdRr : FSTLD<1, 0b10,
                        (outs LDSTPtrReg
                         : $base_wb),
                        (ins LDSTPtrReg
                         : $ptrreg, GPR8
                         : $reg, i8imm
                         : $offs),
                        "st\t-$ptrreg, $reg", [(set i16
                                                : $base_wb, (pre_store GPR8
                                                             : $reg, i16
                                                             : $ptrreg, imm
                                                             : $offs))]>,
                  Requires<[HasSRAM]>;

  // STW -P, Rr+1:Rr
  // Stores the value of Rr into the location addressed by pointer P.
  // Pre decrements P.
  //
  // Expands to:
  // st -P, Rr+1
  // st -P, Rr
  def STWPtrPdRr : Pseudo<(outs PTRREGS
                           : $base_wb),
                          (ins PTRREGS
                           : $ptrreg, DREGS
                           : $reg, i8imm
                           : $offs),
                          "stw\t-$ptrreg, $reg", [(set PTRREGS
                                                   : $base_wb, (pre_store i16
                                                                : $reg, i16
                                                                : $ptrreg, imm
                                                                : $offs))]>,
                   Requires<[HasSRAM]>;
}

// Store indirect with displacement operations.
// STD P+q, Rr
// Stores the value of Rr into the location addressed by pointer P with a
// displacement of q. Does not modify P.
def STDPtrQRr : FSTDLDD<1, (outs),
                        (ins memri
                         : $memri, GPR8
                         : $reg),
                        "std\t$memri, $reg", [(store i8
                                               : $reg, addr
                                               : $memri)]>,
                Requires<[HasSRAM]>;

// STDW P+q, Rr+1:Rr
// Stores the value of Rr into the location addressed by pointer P with a
// displacement of q. Does not modify P.
//
// Expands to:
// std P+q,   Rr
// std P+q+1, Rr+1
def STDWPtrQRr : Pseudo<(outs),
                        (ins memri
                         : $memri, DREGS
                         : $src),
                        "stdw\t$memri, $src", [(store i16
                                                : $src, addr
                                                : $memri)]>,
                 Requires<[HasSRAM]>;

// SWAP Rd
// Swaps the high and low nibbles in a register.
let Constraints =
    "$src = $rd" in def SWAPRd : FRd<0b1001, 0b0100010,
                                     (outs GPR8
                                      : $rd),
                                     (ins GPR8
                                      : $src),
                                     "swap\t$rd", [(set i8
                                                    : $rd, (I8085Swap i8
                                                            : $src))]>;

// IO register bit set/clear operations.

// def CBR : InstAlias<"cbr\t$rd, $k", (ANDIRdK LD8 : $rd, imm_com8 : $k), 0>;

// CLR Rd
// Alias for EOR Rd, Rd
// -------------
// Clears all bits in a register.
// def CLR : InstAlias<"clr\t$rd", (EORRdRr GPR8 : $rd, GPR8 : $rd)>;




//===----------------------------------------------------------------------===//
// Pseudo instructions for later expansion
//===----------------------------------------------------------------------===//

//: TODO: Optimize this for wider types AND optimize the following code
//       compile int foo(char a, char b, char c, char d) {return d+b;}
//       looks like a missed sext_inreg opportunity.
def SEXT
    : ExtensionPseudo<(outs DREGS
                       : $dst),
                      (ins GPR8
                       : $src),
                      "sext\t$dst, $src",
                      []>;

def ZEXT
    : ExtensionPseudo<(outs DREGS
                       : $dst),
                      (ins GPR8
                       : $src),
                      "zext\t$dst, $src",
                      [(set i16
                        : $dst, (zext i8
                                 : $src)),
                       (implicit SREG)]>;

// This pseudo gets expanded into a movw+adiw thus it clobbers SREG.
let Defs = [SREG],
    hasSideEffects = 0 in def FRMIDX : Pseudo<(outs DLDREGS
                                               : $dst),
                                              (ins DLDREGS
                                               : $src, i16imm
                                               : $src2),
                                              "frmidx\t$dst, $src, $src2", []>;


// SP read/write pseudos.
let hasSideEffects = 0 in {
  let Uses = [SP] in def SPREAD : Pseudo<(outs DREGS
                                          : $dst),
                                         (ins GPRSP
                                          : $src),
                                         "spread\t$dst, $src", []>;

  let Defs = [SP] in def SPWRITE : Pseudo<(outs GPRSP
                                           : $dst),
                                          (ins DREGS
                                           : $src),
                                          "spwrite\t$dst, $src", []>;
}




// ------------------------------------- NEW ---------------------------------------

let Constraints = "$src = $rd", Defs=[SREG]  in {

  def SUB_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "SUB_8\t$rd, $rr",
              [(set i8: $rd, (sub i8: $src, i8: $rr))]>;

  def SUB_16
      : I8085Pseudo<(outs GR16: $rd),
              (ins GR16: $src, GR16: $rr),
              "SUB_16\t$rd, $rr",
              [(set i16: $rd, (sub i16: $src, i16: $rr))]>;

  def ADD_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "ADD_8\t$rd, $rr",
              [(set i8: $rd, (add i8: $src, i8: $rr))]>;

  def ADD_16
      : I8085Pseudo<(outs GR16: $rd),
              (ins GR16: $src, GR16: $rr),
              "ADD_16\t$rd, $rr",
              [(set i16: $rd, (add i16: $src, i16: $rr))]>;
  def SHL_8
      : I8085InserterPseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "SHL_8\t$rd, $rr",
              [(set i8: $rd, (shl i8: $src, i8: $rr))]>;
  def SRA_8
      : I8085InserterPseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "SRA_8\t$rd, $rr",
              [(set i8: $rd, (sra i8: $src, i8: $rr))]>;
}

let Defs=[SREG,A], Uses=[A] in def SUB : I8085Pseudo<(outs), (ins GR8: $rs), "SUB $rs", []>;
let Defs=[SREG,A], Uses=[A] in def SBB : I8085Pseudo<(outs), (ins GR8: $rs), "SBB $rs",[]>;
def SUB_M : I8085Pseudo<(outs), (ins), "SUB M", []>;
let Defs=[SREG,A], Uses=[A] in def ADD : I8085Pseudo<(outs), (ins GR8: $rs), "ADD $rs", []>;
let Defs=[SREG,A], Uses=[A] in def ADC : I8085Pseudo<(outs), (ins GR8: $rs), "ADC $rs", []>;
def ADD_M : I8085Pseudo<(outs), (ins), "ADD M", []>;
let Defs=[SREG,A], Uses=[A] in def ADI : I8085Pseudo<(outs), (ins i8imm: $val), "ADI $val", []>;


let Constraints = "$src = $rd", Defs=[SREG]  in {

  def AND_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "AND_8\t$rd, $rr",
              [(set i8: $rd, (and i8: $src, i8: $rr))]>;

  def AND_16
      : I8085Pseudo<(outs GR16: $rd),
              (ins GR16: $src, GR16: $rr),
              "AND_16\t$rd, $rr",
              [(set i16: $rd, (and i16: $src, i16: $rr))]>;

  def OR_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "OR_8\t$rd, $rr",
              [(set i8: $rd, (or i8: $src, i8: $rr))]>;

  def OR_16
      : I8085Pseudo<(outs GR16: $rd),
              (ins GR16: $src, GR16: $rr),
              "OR_16\t$rd, $rr",
              [(set i16: $rd, (or i16: $src, i16: $rr))]>;

  def XOR_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, GR8: $rr),
              "XOR_8\t$rd, $rr",
              [(set i8: $rd, (xor i8: $src, i8: $rr))]>;

  def XOR_16
      : I8085Pseudo<(outs GR16: $rd),
              (ins GR16: $src, GR16: $rr),
              "XOR_16\t$rd, $rr",
              [(set i16: $rd, (xor i16: $src, i16: $rr))]>;

  def ANDI_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, i8imm: $val),
              "ANDI_8\t$rd, $val",
              [(set i8: $rd, (and i8: $src, imm: $val))]>;
  def ORI_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, i8imm: $val),
              "ORI_8\t$rd, $val",
              [(set i8: $rd, (or i8: $src, imm: $val))]>;
  def XORI_8
      : I8085Pseudo<(outs GR8: $rd),
              (ins GR8: $src, i8imm: $val),
              "XORI_8\t$rd, $val",
              [(set i8: $rd, (xor i8: $src, imm: $val))]>;                                                                                          
}


let Defs=[SREG,A], Uses=[A] in def DCR : I8085Pseudo<(outs), (ins GR8: $rs), "DCR $rs", []>;
let Defs=[SREG,A], Uses=[A] in def INR : I8085Pseudo<(outs), (ins GR8: $rs), "INR $rs", []>;


let Defs=[SREG,A], Uses=[A] in def RL_8 : I8085Pseudo<(outs), (ins GR8: $rs), "RL_8 $rs", []>;
let Defs=[SREG,A], Uses=[A] in def RR_8 : I8085Pseudo<(outs), (ins GR8: $rs), "RR_8 $rs", []>;

def RLC : I8085Pseudo<(outs), (ins), "RLC", []>;
def RRC : I8085Pseudo<(outs), (ins), "RRC", []>;


let Defs=[SREG,A], Uses=[A] in def ANA : I8085Pseudo<(outs), (ins GR8: $rs), "ANA $rs", []>;
let Defs=[SREG,A], Uses=[A] in def ORA : I8085Pseudo<(outs), (ins GR8: $rs), "ORA $rs", []>;
let Defs=[SREG,A], Uses=[A] in def XRA : I8085Pseudo<(outs), (ins GR8: $rs), "XRA $rs", []>;

let Defs=[SREG,A], Uses=[A] in def ANI : I8085Pseudo<(outs), (ins i8imm: $val), "ANI $val", []>;
let Defs=[SREG,A], Uses=[A] in def ORI : I8085Pseudo<(outs), (ins i8imm: $val), "ORI $val", []>;
let Defs=[SREG,A], Uses=[A] in def XRI : I8085Pseudo<(outs), (ins i8imm: $val), "XRI $val", []>;


def MVI : I8085Pseudo<
                    (outs GR8: $rd),
                    (ins i8imm: $k),
                    "MVI\t$rd, $k", [(set i8: $rd, imm: $k)]>;


def MOV : I8085Pseudo<
                    (outs GR8: $rd),
                    (ins GR8: $rs),
                    "MOV\t$rd, $rs", [(set i8: $rd, i8: $rs)]>;


def mspi : Operand<iPTR> { 
  let MIOperandInfo = (ops GPRSP, i16imm); 
  let PrintMethod = "printMemri";
  let EncoderMethod = "encodeMemri";
}


def LXI : I8085Pseudo<(outs GR8: $rd), (ins i16imm: $k), "LXI $rd, $k", []>;                  

let Uses = [H,L] in {
  let Defs=[SREG,H,L] in def DAD : I8085Pseudo<(outs), (ins), "DAD\tSP", []>;

  let Defs=[SP] in def SPHL : I8085LegalInst8<0b11111001,(outs), (ins), "SPHL", []>;                          

  def SHLD : I8085Pseudo<(outs), (ins i16imm: $k), "SHLD $k", []>;   

  def LHLD : I8085Pseudo<(outs), (ins i16imm: $k), "LHLD $k", []>;                      
  
  def MOV_M  : I8085Pseudo<(outs), (ins GR8: $rs), "MOV M, $rs", []>;

  def MOV_FROM_M  : I8085Pseudo<(outs GR8: $rd), (ins), "MOV $rd, M", []>;  
}          

let Defs=[SP], Uses=[SP] in {

def GROW_STACK_BY 
    : I8085Pseudo<(outs),
            (ins i16imm: $amount),
            "GROW_STACK_BY $amount",
            []>;

def SHRINK_STACK_BY 
    : I8085Pseudo<(outs),
            (ins i16imm: $amount),
            "SHRINK_STACK_BY $amount",
            []>;        
}       

let Uses=[SP] in {
def LOAD_16 : I8085Pseudo<(outs GR16: $dst),
                       (ins i16imm: $src),
                       "LOAD_16\t$dst, $src", [(set i16
                                             : $dst, imm
                                             : $src)]>; 

def STORE_16 : I8085Pseudo<(outs),
                            (ins mspi
                             : $dst, GR16
                             : $src),
                            "STORE_16\t$dst, $src", [(store i16
                                                     : $src, addr
                                                     : $dst)]>;

def STORE_8 : I8085Pseudo<(outs),
                        (ins mspi: $memri,
                         GR8: $reg),
                        "STORE\t$memri, $reg", [(store i8: $reg, addr: $memri)]>;


def LOAD_16_WITH_ADDR : I8085Pseudo<(outs GR16: $dst),
                       (ins mspi: $src),
                       "LOAD\t$dst, $src", [(set i16: $dst, (load addr: $src))]>;    

def LOAD_8_WITH_ADDR : I8085Pseudo<(outs GR8: $dst),
                       (ins mspi: $src),
                       "LOAD\t$dst, $src", [(set i8: $dst, (load addr: $src))]>;


def STORE_8_AT_OFFSET_WITH_SP : I8085Pseudo<(outs),
                       (ins GR8: $src, i16imm: $offset),
                       "STORE_8_SP_AT_OFFSET\t$offset, $src", [(store i8: $src, imm: $offset)]>;

def STORE_16_AT_OFFSET_WITH_SP : I8085Pseudo<(outs),
                       (ins GR16: $src, i16imm: $offset),
                       "STORE_16_SP_AT_OFFSET\t$offset, $src", [(store i16: $src, imm: $offset)]>;                       

}


def addressTarget : Operand<OtherVT> {
  let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeRelCondBrTarget<I8085::fixup_13_pcrel>";
}

let isCall = 1 in {                                                                                                 
let Uses = [SP] in def CALL : I8085Pseudo<(outs), (ins rcalltarget_13:$k),
                                       "CALL $k", [(I8085call imm:$k)]>;
}

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
  def RET : I8085LegalInst8<0b11001001, (outs), (ins), "RET", [(I8085retflag)]>;
}

def : Pat<(I8085call(i16 tglobaladdr:$dst)), (CALL tglobaladdr:$dst)>;
def : Pat<(I8085call(i16 texternalsym:$dst)), (CALL texternalsym:$dst)>;



def imm16_Neg_Form : SDNodeXForm<imm, [{
                    return CurDAG->getTargetConstant(-N->getAPIntValue(),
                                                     SDLoc(N), MVT::i16);
                  }]>;


def imm8_Neg_Form : SDNodeXForm<imm, [{
                    return CurDAG->getTargetConstant(-N->getAPIntValue(),
                                                     SDLoc(N), MVT::i8);
                  }]>;


def IsNeg8 : ImmLeaf<i8, [{
  return Imm < 0;
}], imm8_Neg_Form>;

def IsNeg16 : ImmLeaf<i16, [{
  return Imm < 0;
}], imm16_Neg_Form>;


def : Pat<(add i8: $src1, (IsNeg8 :$src2)),(SUB_8 i8: $src1, (MVI (imm8_Neg_Form imm: $src2)))>;
def : Pat<(add i16: $src1, (IsNeg16 :$src2)),(SUB_16 i16: $src1, (LOAD_16 (imm16_Neg_Form imm: $src2)))>;

def : Pat<(add i8: $src1, (imm :$src2)),(ADD_8 i8: $src1, (MVI imm: $src2))>;
def : Pat<(add i16: $src1, (imm :$src2)),(ADD_16 i16: $src1, (LOAD_16 imm: $src2))>;


include "I8085CondBrInstrInfo.td"


def SEXT8TO16
    : I8085Pseudo<(outs GR16
                       : $dst),
                      (ins GR8
                       : $src),
                      "sext8to16\t$dst, $src",
                      [(set i16
                        : $dst, (sext i8
                                 : $src))]>;