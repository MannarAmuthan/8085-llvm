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


class I8085LegalInst8<bits<8> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : I8085Inst8<outs, ins, asmstr, pattern> {
  let Inst = opcode;
}

