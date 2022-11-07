//===-- I8085.td - Describe the I8085 Target Machine ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//
// This is the top level entry point for the I8085 target.
//===---------------------------------------------------------------------===//

//===---------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===---------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===---------------------------------------------------------------------===//
// Register File Description
//===---------------------------------------------------------------------===//

include "I8085RegisterInfo.td"

//===---------------------------------------------------------------------===//
// Instruction Descriptions
//===---------------------------------------------------------------------===//

include "I8085InstrInfo.td"


def I8085InstrInfo : InstrInfo {
  let useDeprecatedPositionallyEncodedOperands = true;
}

//===---------------------------------------------------------------------===//
// Calling Conventions
//===---------------------------------------------------------------------===//

include "I8085CallingConv.td"

//===---------------------------------------------------------------------===//
// Assembly Printers
//===---------------------------------------------------------------------===//

def I8085AsmWriter : AsmWriter {
  string AsmWriterClassName = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===---------------------------------------------------------------------===//
// Assembly Parsers
//===---------------------------------------------------------------------===//

def I8085AsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 1;
  let ShouldEmitMatchRegisterAltName = 1;
}

def I8085AsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "$";
  string TokenizingCharacters = "+";
}

//===---------------------------------------------------------------------===//
// Target Declaration
//===---------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"i8085", []>;


def I8085 : Target {
  let InstructionSet = I8085InstrInfo;
  let AssemblyWriters = [I8085AsmWriter];

  let AssemblyParsers = [I8085AsmParser];
  let AssemblyParserVariants = [I8085AsmParserVariant];
}
