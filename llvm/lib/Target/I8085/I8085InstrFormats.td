//===-- I8085InstrInfo.td - I8085 Instruction Formats ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// I8085 Instruction Format Definitions.
//
//===----------------------------------------------------------------------===//

// A generic I8085 instruction.
class I8085Inst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  let Namespace = "I8085";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  field bits<32> SoftFail = 0;
}


class I8085Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst<outs, ins, asmstr, pattern> {

    let isPseudo = 1;
    let isCodeGenOnly = 1;

}


class I8085InserterPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst<outs, ins, asmstr, pattern> {
    
    let usesCustomInserter = 1;
    let isPseudo = 1;
    let isCodeGenOnly = 1;
}


class I8085Inst8<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst<outs, ins, asmstr, pattern> {
  field bits<8> Inst;
  let Size = 1;
}

class I8085Inst16<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst<outs, ins, asmstr, pattern> {
  field bits<16> Inst;
  let Size = 2;
}


//===----------------------------------------------------------------------===//
// 1 Byte Instructions with 0 operands
//===----------------------------------------------------------------------===//

class F1<bits<8> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst8<outs, ins, asmstr, pattern> {
  let Inst = opcode;
}

//===----------------------------------------------------------------------===//
// MOV Inst Format
// |01DD|DSSS

// D - Dest Reg
// S - Src Reg
//===----------------------------------------------------------------------===//
class FMOV<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst8<outs, ins, asmstr, pattern> {
  
  bits<4> rd;
  bits<4> rs;

  let Inst{7-6} = 0b01;
  let Inst{5-3} = rd{2-0};
  let Inst{2-0} = rs{2-0};

}

//===----------------------------------------------------------------------===//
// MOV From memory and to memory - Inst Formats
// To memory - |0111|0SSS
// From memory - |01DD|D110

// D - Dest Reg
// S - Src Reg
//===----------------------------------------------------------------------===//
class FMOV_F_M<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst8<outs, ins, asmstr, pattern> {
  bits<4> rd;
  let Inst{7-6} = 0b01;
  let Inst{5-3} = rd{2-0};
  let Inst{2-0} = 0b110;
}

class FMOV_T_M<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst8<outs, ins, asmstr, pattern> {
  bits<4> rs;
  let Inst{7-3} = 0b01110;
  let Inst{2-0} = rs{2-0};
}

//===----------------------------------------------------------------------===//
// MVI Inst Format
// |00DD|D110|KKKK|KKKK

// D - Dest Reg
// K - Immediate Value
//===----------------------------------------------------------------------===//
class FMVI<dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst16<outs, ins, asmstr, pattern> {
  
  bits<4> rd;
  bits<8> k;

  let Inst{15-14} = 0b00;
  let Inst{13-11} = rd{2-0};
  let Inst{10-8} = 0b110;
  let Inst{7-0} = k;
}

//===----------------------------------------------------------------------===//
// ADD,ADC,SUB,SBB,ANA,XRA,ORA,CMP Instruction Formats
// |xxxx|xSSS|

// x - Instruction Specific Opcode
//          10000 - ADD
//          10001 - ADC
//          10010 - SUB
//          10011 - SBB
//          10100 - ANA
//          10101 - XRA
//          10110 - ORA
//          10111 - CMP
// S - Operand Register
//===----------------------------------------------------------------------===//
class F2<bits<5> opcode,dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst8<outs, ins, asmstr, pattern> {
  
  bits<4> rs;

  let Inst{7-3} = opcode{4-0};
  let Inst{2-0} = rs{2-0};
}

