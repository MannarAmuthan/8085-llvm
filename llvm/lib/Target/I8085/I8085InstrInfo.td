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


// Address operand for `SP+imm` used by STD{W}SPQRr
def memspi : Operand<iPTR> { 
  let MIOperandInfo = (ops GPRSP, i16imm); 
}

def imm16Addr : Operand<i16> {
  let EncoderMethod = "encodeImm<I8085::fixup_16, 0>";
}



// Addressing mode pattern reg+imm6
def addr : ComplexPattern<iPTR, 2, "SelectAddr", [], [SDNPWantRoot]>;

// AsmOperand class for a pointer register.
// Used with the LD/ST family of instructions.
// See FSTLD in I8085InstrFormats.td
// def PtrRegAsmOperand : AsmOperandClass { let Name = "Reg"; }

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
  def ADJCALLSTACKDOWN : I8085Pseudo<(outs),
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
  def ADJCALLSTACKUP : I8085Pseudo<(outs),
               (ins i16imm
                : $amt1, i16imm
                : $amt2),
               "#ADJCALLSTACKUP", [(I8085callseq_end timm
                                    : $amt1, timm
                                    : $amt2)]>;
}

//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// I8085 Arithmetic Instructions list
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//

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

  def SHL_16
      : I8085InserterPseudo<(outs GR16: $rd),
              (ins GR16: $src, GR8: $rr),
              "SHL_16\t$rd, $rr",
              [(set i16: $rd, (shl i16: $src, i8: $rr))]>;

  def SRA_16
      : I8085InserterPseudo<(outs GR16: $rd),
              (ins GR16: $src, GR8: $rr),
              "SRA_16\t$rd, $rr",
              [(set i16: $rd, (sra i16: $src, i8: $rr))]>;            
}

let Defs=[SREG,A], Uses=[A] in def SUB : F2<0b10010,(outs), (ins GR8: $rs), "SUB $rs", []>;
let Defs=[SREG,A], Uses=[A] in def SBB : F2<0b10011,(outs), (ins GR8: $rs), "SBB $rs",[]>;
let Uses=[H,L],Defs=[SREG,A] in def SUB_M : F1<0b10010110,(outs), (ins), "SUB M", []>;
let Uses=[H,L],Defs=[SREG,A] in def SBB_M : F1<0b10011110,(outs), (ins), "SBB M", []>;
let Defs=[SREG,A], Uses=[A] in def ADD : F2<0b10000,(outs), (ins GR8: $rs), "ADD $rs", []>;
let Defs=[SREG,A], Uses=[A] in def ADC : F2<0b10001,(outs), (ins GR8: $rs), "ADC $rs", []>;

let Uses=[H,L] in def ADD_M : F1<0b10000110,(outs), (ins), "ADD M", []>;
let Uses=[H,L] in def ADC_M : F1<0b10001110,(outs), (ins), "ADC M", []>;
let Uses=[H,L] in def ANA_M : F1<0b10100110,(outs), (ins), "ANA M", []>;
let Uses=[H,L] in def ORA_M : F1<0b10110110,(outs), (ins), "ORA M", []>;


let Uses=[H,L],Defs=[SREG,A] in def CMP_M : F1<0b10111101,(outs), (ins), "CMP M", []>;
let Uses=[H,L],Defs=[SREG,A] in def XRA_M : F1<0b10101110,(outs), (ins), "XRA M", []>;

let Defs=[SREG,A], Uses=[A] in def ADI : F5<0b11000110,(outs), (ins i8imm: $k), "ADI $k", []>;
let Defs=[SREG,A], Uses=[A] in def ACI : F5<0b11001110,(outs), (ins i8imm: $k), "ACI $k", []>;
let Defs=[SREG,A], Uses=[A] in def SBI : F5<0b11011110,(outs), (ins i8imm: $k), "SBI $k", []>;
let Defs=[SREG,A], Uses=[A] in def SUI : F5<0b11010110,(outs), (ins i8imm: $k), "SUI $k", []>;

let Constraints = "$rs = $rd", Defs=[SREG]  in {

def DCR : F3<0b1,(outs GR8: $rd), (ins GR8: $rs), "DCR $rs", []>;

def INR : F3<0b0,(outs GR8: $rd), (ins GR8: $rs), "INR $rs", []>;

}

//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// I8085 Logical Instructions list
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//


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

let Defs=[SREG,A], Uses=[A] in def ANA : F2<0b10100,(outs), (ins GR8: $rs), "ANA $rs", []>;
let Defs=[SREG,A], Uses=[A] in def ORA : F2<0b10110,(outs), (ins GR8: $rs), "ORA $rs", []>;
let Defs=[SREG,A], Uses=[A] in def XRA : F2<0b10101,(outs), (ins GR8: $rs), "XRA $rs", []>;

let Defs=[SREG,A], Uses=[A] in def ANI : F5<0b11100110,(outs), (ins i8imm: $k), "ANI $k", []>;
let Defs=[SREG,A], Uses=[A] in def ORI : F5<0b11110110,(outs), (ins i8imm: $k), "ORI $k", []>;
let Defs=[SREG,A], Uses=[A] in def XRI : F5<0b11101110,(outs), (ins i8imm: $k), "XRI $k", []>;


//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// I8085 Bitwise Instructions list
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//




let Constraints = "$rs = $rd", Defs=[SREG]  in {
  let Defs=[SREG,A], Uses=[A] in def RL_8 : I8085Pseudo<(outs GR8: $rd), (ins GR8: $rs), "RL_8 $rs", []>;
  let Defs=[SREG,A], Uses=[A] in def RR_8 : I8085Pseudo<(outs GR8: $rd), (ins GR8: $rs), "RR_8 $rs", []>;

  let Defs=[SREG,A], Uses=[A] in def RL_16 : I8085Pseudo<(outs GR16: $rd), (ins GR16: $rs), "RL_16 $rs", []>;
  let Defs=[SREG,A], Uses=[A] in def RR_16 : I8085Pseudo<(outs GR16: $rd), (ins GR16: $rs), "RR_16 $rs", []>;
}

def RLC : F1<0b00000111,(outs), (ins), "RLC", []>;
def RRC : F1<0b00001111,(outs), (ins), "RRC", []>;

def RAL : F1<0b00010111,(outs), (ins), "RAL", []>;
def RAR : F1<0b00011111,(outs), (ins), "RAR", []>;



//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//
// I8085 Data movement operations
//===----------------------------------------------------------------------===//
//===----------------------------------------------------------------------===//


def MVI : FMVI<(outs GR8: $rd),
                    (ins i8imm: $k),
                    "MVI $rd, $k", [(set i8: $rd, imm: $k)]>;


def MOV : FMOV<(outs GR8: $rd),
                    (ins GR8: $rs),
                    "MOV $rd, $rs", []>;


def mspi : Operand<iPTR> { 
  let MIOperandInfo = (ops GPRSP, i16imm); 
}


def LXI : F4<0b0001,(outs GR8: $rd), (ins i16imm: $k), "LXI $rd, $k", []>;
def INX : F8<0b0011,(outs GR8: $rd), (ins), "INX $rd", []>;

let Uses = [H,L] in {
  let Defs=[SREG,H,L] in def DAD : F7<(outs), (ins GR16: $rs), "DAD $rs", []>;

  let Defs=[SP] in def SPHL : F1<0b11111001,(outs), (ins), "SPHL", []>;                          

  def SHLD : F1<0b00100010,(outs), (ins i16imm: $k), "SHLD $k", []>;   

  def LHLD : F1<0b00101010,(outs), (ins i16imm: $k), "LHLD $k", []>;                      
  
  let Uses=[H,L] in def MOV_M  : FMOV_T_M<(outs), (ins GR8: $rs), "MOV M, $rs", []>;

  let Uses=[H,L] in def MOV_FROM_M  : FMOV_F_M<(outs GR8: $rd), (ins), "MOV $rd, M", []>;

  def MVI_M : F1<0b00111110,(outs), (ins i8imm: $k), "MVI M, $k", []>; 
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
                       "LOAD_16 $dst, $src", [(set i16
                                             : $dst, imm
                                             : $src)]>; 

def STORE_16 : I8085Pseudo<(outs),
                            (ins mspi: $dst, GR16: $src),
                            "STORE_16 $dst, $src", [(store i16: $src, addr: $dst)]>;

def STORE_8 : I8085Pseudo<(outs),
                        (ins mspi: $memri,
                         GR8: $reg),
                        "STORE $memri, $reg", [(store i8: $reg, addr: $memri)]>;


def LOAD_16_WITH_ADDR : I8085Pseudo<(outs GR16: $dst),
                       (ins mspi: $src),
                       "LOAD $dst, $src", [(set i16: $dst, (load addr: $src))]>;    

def LOAD_8_WITH_ADDR : I8085Pseudo<(outs GR8: $dst),
                       (ins mspi: $src),
                       "LOAD $dst, $src", [(set i8: $dst, (load addr: $src))]>;


def STORE_8_AT_OFFSET_WITH_SP : I8085Pseudo<(outs),
                       (ins GR8: $src, i16imm: $offset),
                       "STORE_8_SP_AT_OFFSET\t$offset, $src", [(store i8: $src, imm: $offset)]>;

def STORE_16_AT_OFFSET_WITH_SP : I8085Pseudo<(outs),
                       (ins GR16: $src, i16imm: $offset),
                       "STORE_16_SP_AT_OFFSET\t$offset, $src", [(store i16: $src, imm: $offset)]>;                       

}

let Uses=[SP] in {

def LOAD_8_WITH_IMM_ADDR : I8085Pseudo<(outs GR8: $dst),
                       (ins imm16Addr: $src),
                       "LOAD_8_ADDR $dst, $src", []>;  

def LOAD_16_WITH_IMM_ADDR : I8085Pseudo<(outs GR16: $dst),
                       (ins imm16Addr: $src),
                       "LOAD_16_ADDR $dst, $src", []>;

def LOAD_8_ADDR_CONTENT : I8085Pseudo<(outs GR8: $dst),
                       (ins GR16: $src),
                       "LOAD_8_ADDR_CONTENT $dst, $src", [(set i8: $dst, (load i16: $src))]>;

def LOAD_16_ADDR_CONTENT : I8085Pseudo<(outs GR16: $dst),
                       (ins GR16: $src),
                       "LOAD_16_ADDR_CONTENT $dst, $src", [(set i16: $dst, (load i16: $src))]>;
}  


def addressTarget : Operand<OtherVT> {
  // let PrintMethod = "printPCRelImm";
  let EncoderMethod = "encodeCallTarget<I8085::fixup_16, 0>";
}

let isCall = 1 in {                                                                                                 
let Uses = [SP] in def CALL : F6<0b11001101,(outs), (ins imm16Addr:$k),
                                       "CALL $k", [(I8085call imm:$k)]>;
}

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
  def RET : F1<0b11001001, (outs), (ins), "RET", [(I8085retflag)]>;
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


def SEXT8TO16
    : I8085Pseudo<(outs GR16
                       : $dst),
                      (ins GR8
                       : $src),
                      "sext8to16 $dst, $src",
                      [(set i16
                        : $dst, (sext i8
                                 : $src))]>;

def ZEXT8TO16
    : I8085Pseudo<(outs GR16
                       : $dst),
                      (ins GR8
                       : $src),
                      "zext8to16 $dst, $src",
                      [(set i16
                        : $dst, (zext i8
                                 : $src))]>;                               

def AEXT8TO16
    : I8085Pseudo<(outs GR16
                       : $dst),
                      (ins GR8
                       : $src),
                      "aext8to16 $dst, $src",
                      [(set i16
                        : $dst, (anyext i8
                                 : $src))]>;   


def TRUNC16TO8
    : I8085Pseudo<(outs GR8: $dst),
                      (ins GR16: $src),
                      "trunc16to8 $dst, $src",
                      [(set i8
                        : $dst, (trunc i16
                                 : $src))]>;                                 


def : Pat<(i16(load(I8085Wrapper tglobaladdr
                    : $dst))),
          (LOAD_16_WITH_IMM_ADDR tglobaladdr
           : $dst)>;

def : Pat<(i8(load(I8085Wrapper tglobaladdr
                   : $dst))),
          (LOAD_8_WITH_IMM_ADDR tglobaladdr
           : $dst)>;

def : Pat<(i16(I8085Wrapper tglobaladdr : $dst)), 
          (LOAD_16_WITH_IMM_ADDR tglobaladdr : $dst)>;


let Defs = [SREG],
    hasSideEffects = 0 in  def FRMIDX : I8085Pseudo<(outs GR16: $dst),
                                                    (ins mspi: $memri),
                                                    "FRMIDX\t$dst, $memri", []>;

                                              
include "I8085CondBrInstrInfo.td"

include "I8085InstrInfo32.td"